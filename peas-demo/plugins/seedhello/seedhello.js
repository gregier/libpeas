var Gtk = imports.gi.Gtk;

var LABEL_STRING = "Seed Says Hello Too!";

print("LABEL_STRING=" +  LABEL_STRING);

activatable_extension = {
  activate: function() {
    print("SeedHelloPlugin.activate");
    this.object._seedhello_label = new Gtk.Label({ label: LABEL_STRING });
    this.object._seedhello_label.show();
    this.object.get_child().pack_start(this.object._seedhello_label);
  },
  deactivate: function() {
    print("SeedHelloPlugin.deactivate");
    this.object.get_child().remove(this.object._seedhello_label);
    this.object._seedhello_label.destroy();
  },
  update_state: function() {
    print("SeedHelloPlugin.update_state");
  }
};

configurable_extension = {
  create_configure_widget: function () {
    return new Gtk.Label({ label: "Example of configuration dialog for a Seed plugin" });
  }
};

extensions = {
  'PeasActivatable': activatable_extension,
  'PeasUIConfigurable': configurable_extension,
};
