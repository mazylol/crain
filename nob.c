#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "./"
#define BINARY_NAME "crain"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    // 1. Configuration
    const char *cc = "clang";
    const char *cflags[] = {"-Wall", "-Wextra", "-g"}; // Add common flags here
    const char *libs[]   = {"-lncurses"};

    // List of your source files
    const char *sources[] = {
        SRC_FOLDER "main.c",
        SRC_FOLDER "util.c",
        SRC_FOLDER "arg.c"
    };

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Nob_Cmd cmd = {0};
    Nob_File_Paths object_files = {0};
    Nob_Procs procs = {0}; // Used for async parallel compilation

    // 2. Compile individual source files to object files
    for (size_t i = 0; i < NOB_ARRAY_LEN(sources); ++i) {
        const char *src_path = sources[i];
        
        // Generate a unique object file path (e.g., "build/main.c.o")
        // We include the .c extension in the name to avoid conflicts if you have src/main.c and src/main/lib.c
        const char *obj_path = nob_temp_sprintf("%s%s.o", BUILD_FOLDER, nob_path_name(src_path));
        
        // Add to the list of objects for the linker later
        nob_da_append(&object_files, obj_path);

        // Check if the source needs recompilation
        if (nob_needs_rebuild1(obj_path, src_path)) {
            cmd.count = 0;
            nob_cmd_append(&cmd, cc);
            nob_cmd_append(&cmd, "-c", src_path);
            nob_cmd_append(&cmd, "-o", obj_path);
            nob_da_append_many(&cmd, cflags, NOB_ARRAY_LEN(cflags));
            
            // Run asynchronously (in parallel)
            if (!nob_cmd_run(&cmd, .async = &procs)) return 1;
        }
    }

    // Wait for all compilation processes to finish
    if (!nob_procs_wait(procs)) return 1;

    // 3. Link everything together
    const char *binary_path = BUILD_FOLDER BINARY_NAME;
    
    // Check if the binary needs relinking (if binary is missing OR any object file is newer)
    if (nob_needs_rebuild(binary_path, object_files.items, object_files.count)) {
        cmd.count = 0;
        nob_cmd_append(&cmd , "bear", "--output", BUILD_FOLDER "compile_commands.json", "--");
        nob_cmd_append(&cmd, cc);
        nob_cmd_append(&cmd, "-o", binary_path);
        // Append all object files
        nob_da_append_many(&cmd, object_files.items, object_files.count);
        // Append libraries (usually last)
        nob_da_append_many(&cmd, libs, NOB_ARRAY_LEN(libs));
        
        if (!nob_cmd_run(&cmd)) return 1;
    }

    // Cleanup
    nob_cmd_free(cmd);
    nob_da_free(object_files);
    nob_da_free(procs); // Technically `nob_procs_wait` cleans up, but good habit
    
    return 0;
}
