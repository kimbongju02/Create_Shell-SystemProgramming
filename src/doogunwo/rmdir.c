int remove_directory(const char *dirname) {
    if (rmdir(dirname) != 0) {
        perror("Error removing directory");
        return -1;
    }
    return 0;
}