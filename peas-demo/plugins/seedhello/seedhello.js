var Gtk = imports.gi.Gtk;

var LABEL_STRING = "Seed Says Hello Too!";

print("LABEL_STRING=" +  LABEL_STRING);

activatable_extension = {
  activate: function(win) {
    print("SeedHelloPlugin.activate");
    win._seedhello_label = new Gtk.Label({ label: LABEL_STRING });
    win._seedhello_label.show();
    win.get_child().pack_start(win._seedhello_label);
  },
  deactivate: function(win) {
    print("SeedHelloPlugin.deactivate");
    win.get_child().remove(win._seedhello_label);
    win._seedhello_label.destroy();
  },
  update_state: function(win) {
    print("SeedHelloPlugin.update_state");
  }
};

configurable_extension = {
  create_configure_widget: function () {
    var dialog = new Gtk.Dialog({ title: "Seedhello Config Dialog" });
    var label = new Gtk.Label({ label: "Example of configuration dialog for a Seed plugin" });

    dialog.get_content_area().pack_start(label);
    dialog.add_button(Gtk.STOCK_OK, 1);

    dialog.signal.response.connect(function (w, response_id) {
      print("Dialog button clicked");
      dialog.destroy();
    });

    return dialog;
  }
};

extensions = {
  'PeasActivatable': activatable_extension,
  'PeasUIConfigurable': configurable_extension,
};
