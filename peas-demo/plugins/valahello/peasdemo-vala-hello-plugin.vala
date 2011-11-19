/*
 * Copyright (c) 2010 Abderrahim Kitouni
 * Copyright (c) 2011 Steve Frécinaux
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

using GLib;
using Gtk;
using Peas;
using PeasGtk;

namespace PeasDemo {
  public class ValaHelloPlugin : Peas.ExtensionBase, Peas.Activatable {
    private Gtk.Widget label;
    public GLib.Object object { owned get; construct; }

    public void activate () {
      var window = object as Gtk.Window;

      label = new Gtk.Label ("Hello World from Vala!");
      var box = window.get_child () as Gtk.Box;
      box.pack_start (label);
      label.show ();
    }

    public void deactivate () {
      var window = object as Gtk.Window;

      var box = window.get_child () as Gtk.Box;
      box.remove (label);
    }

    public void update_state () {
    }
  }

  public class ValaPluginConfig : Peas.ExtensionBase, PeasGtk.Configurable {
    public Gtk.Widget create_configure_widget () {
      string text = "This is a configuration dialog for the ValaHello plugin.";
      return new Gtk.Label (text);
    }
  }
}

[ModuleInit]
public void peas_register_types (GLib.TypeModule module) {
  var objmodule = module as Peas.ObjectModule;
  objmodule.register_extension_type (typeof (Peas.Activatable),
                                     typeof (PeasDemo.ValaHelloPlugin));
  objmodule.register_extension_type (typeof (PeasGtk.Configurable),
                                     typeof (PeasDemo.ValaPluginConfig));
}
