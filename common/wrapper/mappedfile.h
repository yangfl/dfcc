#ifndef DFCC_WRAPPER_MAPPEDFILE_H
#define DFCC_WRAPPER_MAPPEDFILE_H

#include <stddef.h>
#include <glib.h>


/**
 * @ingroup Wrapper
 * @brief Map file into memory
 */
struct MappedFile {
  GMappedFile *mapped;
  char *content;
  size_t length;
};


//! @memberof MappedFile
void MappedFile_destroy (struct MappedFile *m);
//! @memberof MappedFile
int MappedFile_init_from_fd (struct MappedFile *m, int fd, GError **error);
//! @memberof MappedFile
int MappedFile_init (struct MappedFile *m, const char *path, GError **error);


#endif /* DFCC_WRAPPER_MAPPEDFILE_H */
