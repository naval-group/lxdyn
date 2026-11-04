/*
 * DirectionalSpreadingBuilder.hpp
 *
 *  Created on: Aug 7, 2014
 *      Author: cady
 */

#ifndef DIRECTIONALSPREADINGBUILDER_HPP_
#define DIRECTIONALSPREADINGBUILDER_HPP_

#include <boost/optional/optional.hpp>

#include <string>
#include <ssc/macros.hpp>
#include <memory>

class WaveDirectionalSpreading;

class DirectionalSpreadingBuilderInterface
{
    public:
        DirectionalSpreadingBuilderInterface() {}
        virtual ~DirectionalSpreadingBuilderInterface(){}
        virtual boost::optional<std::shared_ptr<WaveDirectionalSpreading> > try_to_parse(const std::string& model, const std::string& yaml) const = 0;
};

template <typename T>
class DirectionalSpreadingBuilder : public DirectionalSpreadingBuilderInterface
{
    public:
        DirectionalSpreadingBuilder() : DirectionalSpreadingBuilderInterface(){}
        boost::optional<std::shared_ptr<WaveDirectionalSpreading> > try_to_parse(const std::string& model, const std::string& yaml) const;
};

typedef std::shared_ptr<DirectionalSpreadingBuilderInterface> DirectionalSpreadingBuilderPtr;

#endif /* DIRECTIONALSPREADINGBUILDER_HPP_ */
