extern FILE *output;

void dump_table_assembly     (MonoImage *m);
void dump_table_assemblyref  (MonoImage *m);
void dump_table_class_layout (MonoImage *m);
void dump_table_constant     (MonoImage *m);
void dump_table_customattr   (MonoImage *m);
void dump_table_declsec      (MonoImage *m);
void dump_table_property     (MonoImage *m);
void dump_table_property_map (MonoImage *m);
void dump_table_event        (MonoImage *m);
void dump_table_file         (MonoImage *m);
void dump_table_moduleref    (MonoImage *m);
void dump_table_module       (MonoImage *m);
void dump_table_method       (MonoImage *m);
void dump_table_methodimpl   (MonoImage *m);
void dump_table_methodsem    (MonoImage *m);
void dump_table_field        (MonoImage *m);
void dump_table_manifest     (MonoImage *m);
void dump_table_memberref    (MonoImage *m);
void dump_table_param        (MonoImage *m);
void dump_table_typedef      (MonoImage *m);
void dump_table_typeref      (MonoImage *m);
void dump_table_exported     (MonoImage *m);
void dump_table_nestedclass  (MonoImage *m);
void dump_table_interfaceimpl (MonoImage *m);
void dump_table_field_marshal (MonoImage *m);
void dump_table_genericpar   (MonoImage *m);
void dump_table_methodspec   (MonoImage *m);
void dump_table_parconstraint(MonoImage *m);
void dump_table_implmap      (MonoImage *m);
