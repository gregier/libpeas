# -*- coding: utf-8 -*-

#  Copyright (C) 2014 - Garrett Regier
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

import cProfile
import os
import pstats
import sys
import threading
import weakref


class Hooks(object):
    def __init__(self):
        self.profiling_enabled = os.getenv('PEAS_PYTHON_PROFILE') is not None
        if not self.profiling_enabled:
            return

        sort = os.getenv('PEAS_PYTHON_PROFILE', default='time')
        self.stat_sort = sort.split(';')

        self.stats = None
        self.stats_lock = threading.Lock()

        self.thread_refs = []
        self.thread_local = threading.local()

        threading.setprofile(self.init_thread)

        self.profile = cProfile.Profile()
        self.profile.enable()

    def add_stats(self, profile):
        profile.disable()

        with self.stats_lock:
            if self.stats is None:
                self.stats = pstats.Stats(profile)

            else:
                self.stats.add(profile)

    def init_thread(self, *unused):
        # Only call once per thread
        sys.setprofile(None)

        thread_profile = cProfile.Profile()

        def thread_finished(thread_ref):
            self.add_stats(thread_profile)

            self.thread_refs.remove(thread_ref)

        # Need something to weakref, the
        # current thread does not support it
        thread_ref = set()
        self.thread_local.ref = thread_ref

        self.thread_refs.append(weakref.ref(thread_ref, thread_finished))

        # Only enable the profile at the end
        thread_profile.enable()

    def all_plugins_unloaded(self):
        if not self.profiling_enabled:
            return

        self.add_stats(self.profile)

        with self.stats_lock:
            self.stats.strip_dirs().sort_stats(*self.stat_sort).print_stats()

        # Need to create a new profile to avoid adding the stats twice
        self.profile = cProfile.Profile()
        self.profile.enable()

    def exit(self):
        if not self.profiling_enabled:
            return

        self.profile.disable()


hooks = Hooks()

# ex:ts=4:et:
