#include "h5_tools.h"
#include "h5_version.h"
#include "xdyn/version/version.h"

#include <stdio.h>

/**
 * \brief adds a description to a HDF5 file
 * Below is a Python example of how to use this description
 * \code{.py}
import h5py
with h5py.File('writeFileDescription.h5', 'r') as f:
    for line in f.attrs['Description']:
        print(line.decode().rstrip())
 \endcode
 */
#define NUMBER_OF_ATTRIBUTES 2
#define MAXIMUM_ATTRIBUTE_LENGTH 255
int h5_writeFileDescription(
        char const * const fileName)
{
    herr_t ret = 0;
    hid_t file = h5_getFileId(fileName);
    h5_writeFileDescriptionGivenAFileId(file);
    ret = H5Fclose(file);
    return (ret<0)?0:1;
}

int h5_writeFileDescriptionGivenAFileId(hid_t file)
{
    hid_t root = 0, dataspace = 0, att = 0;
    hid_t filetype = 0;
    hid_t type = 0;
    herr_t ret;
    char string_att[NUMBER_OF_ATTRIBUTES][MAXIMUM_ATTRIBUTE_LENGTH+1] = {{0},{0}};
    hsize_t dims[1] = {NUMBER_OF_ATTRIBUTES};

    snprintf(string_att[0], MAXIMUM_ATTRIBUTE_LENGTH+1, "lxdyn");
    snprintf(string_att[1], MAXIMUM_ATTRIBUTE_LENGTH+1, "Version : %s (%s)", xdyn_version(), xdyn_commit());

    root        = H5Gopen(file, "/", H5P_DEFAULT);
    filetype    = H5Tcopy(H5T_FORTRAN_S1);
    ret         = H5Tset_size(filetype, MAXIMUM_ATTRIBUTE_LENGTH+1);
    type        = H5Tcopy(H5T_C_S1);
    ret         = H5Tset_size(type, MAXIMUM_ATTRIBUTE_LENGTH+1);
    dataspace   = H5Screate_simple(1, dims, dims);
    att         = H5Acreate(root, "Description", filetype, dataspace, H5P_DEFAULT, H5P_DEFAULT);
    ret         = H5Awrite(att, type, string_att);
    ret         = H5Aclose(att);
    ret         = H5Gclose(root);
    ret         = H5Tclose(type);
    ret         = H5Sclose(dataspace);
    return (ret<0)?0:1;
}
