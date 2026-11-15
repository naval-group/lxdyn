/*
 * hdb_parser_internal_data_structures.hpp
 *
 *  Created on: Nov 6, 2014
 *      Author: cady
 */

#ifndef HDB_PARSER_INTERNAL_DATA_STRUCTURES_HPP_
#define HDB_PARSER_INTERNAL_DATA_STRUCTURES_HPP_

#include <string>
#include <vector>

namespace hdb
{
    using Header = std::string;
    using Value = double;
    using Values = std::vector<Value>;
    using ListOfValues = std::vector<Values>;

    template <typename T> struct Key
    {
        Key() :  value() {}

        Header header;
        T      value;
    };

    struct VectorSection
    {
        Header header;
        Values values;
    };

    struct MatrixSection
    {
        Header header;
        ListOfValues values;
    };

    struct SectionWithId
    {
        Header header;
        Value  id{};
        ListOfValues values;
    };

    struct ListOfMatrixSections
    {
        Header               header;
        std::vector<MatrixSection> sections;
    };

    struct ListOfMatrixSectionsWithId
    {
        Header                     header;
        std::vector<SectionWithId> sections_with_id;
    };

    struct AST
    {
        std::vector<Key<std::string> >    string_keys;
        std::vector<Key<double> >         value_keys;
        std::vector<VectorSection>        vector_sections;
        std::vector<MatrixSection>        matrix_sections;
        std::vector<ListOfMatrixSections> lists_of_matrix_sections;
        std::vector<ListOfMatrixSectionsWithId> lists_of_matrix_sections_with_id;
    };
}

#endif /* HDB_PARSER_INTERNAL_DATA_STRUCTURES_HPP_ */
