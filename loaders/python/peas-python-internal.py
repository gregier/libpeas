# -*- coding: utf-8 -*-

#  Copyright (C) 2014-2015 - Garrett Regier
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

import gc
import gettext
import importlib
import os
import signal
import sys
import traceback

from gi.repository import GLib, GObject


# Derive from something not normally caught
class FailedError(BaseException):
    pass


class Hooks(object):
    def __init__(self):
        if not ALREADY_INITIALIZED:
            int_handler = signal.getsignal(signal.SIGINT)

            # Use the default handler instead of raising KeyboardInterrupt
            if int_handler == signal.default_int_handler:
                signal.signal(signal.SIGINT, signal.SIG_DFL)

        # See PySys_SetArgvEx()
        sys.argv = [PRGNAME]
        sys.path.pop(0)

        sys.path.insert(0, PEAS_PYEXECDIR)
        gettext.install(GETTEXT_PACKAGE, PEAS_LOCALEDIR)

        self.__idle_gc = 0
        self.__module_cache = {}
        self.__extension_cache = {}

    @staticmethod
    def failed():
        # This is implemented by the plugin loader
        raise NotImplementedError('Hooks.failed()')

    def load(self, filename, module_dir, module_name):
        try:
            return self.__module_cache[filename]

        except KeyError:
            pass

        if module_name in sys.modules:
            self.__module_cache[filename] = None
            self.failed("Error loading plugin '%s': "
                        "module name '%s' has already been used" %
                        (filename, module_name))

        if module_dir not in sys.path:
            sys.path.insert(0, module_dir)

        try:
            module = importlib.import_module(module_name)

        except:
            module = None
            self.failed("Error importing plugin '%s':\n%s" %
                        (module_name, traceback.format_exc()))

        else:
            self.__extension_cache[module] = {}

        finally:
            self.__module_cache[filename] = module

        return module

    def find_extension_type(self, gtype, module):
        module_gtypes = self.__extension_cache[module]

        try:
            return module_gtypes[gtype]

        except KeyError:
            pass

        for key in module.__dict__:
            value = getattr(module, key)

            try:
                value_gtype = value.__gtype__

            except AttributeError:
                continue

            if GObject.type_is_a(value_gtype, gtype):
                module_gtypes[gtype] = value
                return value

        module_gtypes[gtype] = None
        return None

    def __run_gc(self):
        gc.collect()

        self.__idle_gc = 0
        return False

    def garbage_collect(self):
        # We run the GC right now and we schedule
        # a further collection in the main loop
        gc.collect()

        if self.__idle_gc == 0:
            self.__idle_gc = GLib.idle_add(self.__run_gc)
            GLib.source_set_name_by_id(self.__idle_gc, '[libpeas] run_gc')

    def all_plugins_unloaded(self):
        pass

    def exit(self):
        gc.collect()

        if self.__idle_gc != 0:
            GLib.source_remove(self.__idle_gc)


if os.getenv('PEAS_PYTHON_PROFILE') is not None:
    import cProfile
    import pstats
    import threading
    import weakref


    class Hooks(Hooks):
        def __init__(self):
            super(Hooks, self).__init__()

            sort = os.getenv('PEAS_PYTHON_PROFILE', default='time')
            self.__stat_sort = sort.split(';')

            self.__stats = None
            self.__stats_lock = threading.Lock()

            self.__thread_refs = []
            self.__thread_local = threading.local()

            threading.setprofile(self.__init_thread)

            self.__profile = cProfile.Profile()
            self.__profile.enable()

        def __add_stats(self, profile):
            profile.disable()

            with self.__stats_lock:
                if self.__stats is None:
                    self.__stats = pstats.Stats(profile)

                else:
                    self.__stats.add(profile)

        def __init_thread(self, *unused):
            # Only call once per thread
            sys.setprofile(None)

            thread_profile = cProfile.Profile()

            def thread_finished(thread_ref):
                self.__add_stats(thread_profile)

                self.__thread_refs.remove(thread_ref)

            # Need something to weakref, the
            # current thread does not support it
            thread_ref = set()
            self.__thread_local.ref = thread_ref

            self.__thread_refs.append(weakref.ref(thread_ref,
                                                  thread_finished))

            # Only enable the profile at the end
            thread_profile.enable()

        def all_plugins_unloaded(self):
            super(Hooks, self).all_plugins_unloaded()

            self.__add_stats(self.__profile)

            with self.__stats_lock:
                stats = self.__stats.strip_dirs()
                stats.sort_stats(*self.__stat_sort)
                stats.print_stats()

            # Need to create a new profile to avoid adding the stats twice
            self.__profile = cProfile.Profile()
            self.__profile.enable()

        def exit(self):
            super(Hooks, self).exit()

            self.__profile.disable()


hooks = Hooks()

# ex:ts=4:et:
