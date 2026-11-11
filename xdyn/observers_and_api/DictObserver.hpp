#ifndef DICTOBSERVER_HPP_
#define DICTOBSERVER_HPP_

#include "xdyn/core/Observer.hpp"
#include "xdyn/core/SurfaceElevationGrid.hpp"

#include <sstream>
#include <utility>
#include <map>

using DictMap1 = std::map<std::string, double>;
using DictMap2 = std::map<std::string, std::map<std::string, double> >;

class DictObserver : public Observer
{
    public:
        DictObserver();
        DictObserver(const std::vector<std::string>& d);
        ~DictObserver() override;

    protected:
        void flush_after_initialization() override{}
        void flush_value_during_write() override{}
        void flush_after_write() override;

        using Observer::get_serializer;
        using Observer::get_initializer;

        std::function<void()> get_serializer(const double val, const DataAddressing& address) override;
        std::function<void()> get_initializer(const double val, const DataAddressing& address) override;

        std::function<void()> get_serializer(const SurfaceElevationGrid& val, const DataAddressing& address) override;
        std::function<void()> get_initializer(const SurfaceElevationGrid& val, const DataAddressing& address) override;

        std::stringstream ss;
    private:
        std::stringstream ssSurfaceElevationGrid;
        DictMap1 dictMap1;
        DictMap2 dictMap2;
        bool shouldWeAddAStartingComma;
        void serializeDictMap1();
        void serializeDictMap2();
        void serializeDictMap2Attitudes();
        void serializeDictMap2Wrenches();
        void serializeDictSurfaceElevationGrid();
};

#endif
