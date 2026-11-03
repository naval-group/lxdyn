#include "ssc/kinematics/Kinematics.hpp"
#include "ssc/data_source/DataSource.hpp"
#include "ssc/kinematics/KinematicsException.hpp"
#include "ssc/kinematics/Transform.hpp"
#include "ssc/kinematics/KinematicTree.hpp"

using namespace ssc::data_source;
using namespace ssc::kinematics;

std::string make_transform_name(const std::string& from_frame, const std::string& to_frame);
std::string make_transform_name(const std::string& from_frame, const std::string& to_frame)
{
    return from_frame + " -> " + to_frame;
}

std::string make_transform_name(const std::pair<std::string,std::string>& from_to);
std::string make_transform_name(const std::pair<std::string,std::string>& from_to)
{
    return make_transform_name(from_to.first, from_to.second);
}


class InverseTransformComputer : public DataSourceModule
{
    public:
        InverseTransformComputer(DataSource* const data_source, const std::string& module_name) : DataSourceModule(data_source, module_name), name_of_direct_transform(""), name_of_inverse_transform("")
        {
        }

        InverseTransformComputer(const InverseTransformComputer& rhs, DataSource* const data_source) : DataSourceModule(rhs, data_source), name_of_direct_transform(rhs.name_of_direct_transform), name_of_inverse_transform(rhs.name_of_inverse_transform)
        {
        }

        DataSourceModule* clone() const
        {
            return new InverseTransformComputer(*this);
        }

        DataSourceModule* clone(DataSource* const data_source) const
        {
            return new InverseTransformComputer(*this, data_source);
        }

        void update() const
        {
            const Transform t = ds->get<Transform>(name_of_direct_transform);
            ds->set(name_of_inverse_transform, t.inverse());
        }

        std::string name_of_direct_transform;
        std::string name_of_inverse_transform;
};

class CompositeTransformComputer : public DataSourceModule
{
    public:
        CompositeTransformComputer(DataSource* const data_source, const std::string& module_name) : DataSourceModule(data_source, module_name), transforms(std::vector<std::pair<std::string,std::string> >())
        {
        }

        CompositeTransformComputer(const CompositeTransformComputer& rhs, DataSource* const data_source) : DataSourceModule(rhs, data_source), transforms(std::vector<std::pair<std::string,std::string> >())
        {
        }

        DataSourceModule* clone() const
        {
            return new CompositeTransformComputer(*this);
        }

        DataSourceModule* clone(DataSource* const data_source) const
        {
            return new CompositeTransformComputer(*this, data_source);
        }

        void update() const
        {
            Transform t = ds->get<Transform>(make_transform_name(transforms.front()));
            const std::string name_of_transform = make_transform_name(transforms.front().first,transforms.back().second);

            for (size_t i = 1 ; i < transforms.size() ; ++i)
            {
                t = t * ds->get<Transform>(make_transform_name(transforms[i]));
            }
            ds->set(name_of_transform, t);
        }

        PathType transforms;
};


class Kinematics::Impl
{
    public:
        Impl() : ds(DataSource()), tree(KinematicTree()) {}

        ~Impl(){}
        Impl& operator=(const Impl& rhs)
        {
            if (this!=&rhs)
            {
                ds = rhs.ds;
                tree = rhs.tree;
            }
            return *this;
        }

        Impl(const Impl& rhs) : ds(rhs.ds), tree(rhs.tree)
        {
        }

        void add_inverse_module(const std::string& from_frame, const std::string& to_frame)
        {
            const std::string direct_transform = make_transform_name(from_frame, to_frame);
            InverseTransformComputer computer(&ds, std::string("reverse(")+direct_transform+")");
            computer.name_of_direct_transform = direct_transform;
            computer.name_of_inverse_transform = make_transform_name(to_frame, from_frame);
            ds.add(computer);
            tree.add(from_frame, to_frame);
        }

        void add(const Transform& t)
        {
            if (t.get_from_frame() == t.get_to_frame())
            {
                THROW(__PRETTY_FUNCTION__, KinematicsException, std::string("Cannot add a transform from '") + t.get_from_frame() + "' to itself");
            }
            ds.check_in(__PRETTY_FUNCTION__);
            const std::string direct_transform = make_transform_name(t.get_from_frame(), t.get_to_frame());
            const bool need_to_add_modules = not(ds.has<kinematics::Transform>(direct_transform));
            ds.set(direct_transform, t);
            if (need_to_add_modules)
            {
                add_inverse_module(t.get_from_frame(), t.get_to_frame());
            }
            ds.check_out();
        }

        kinematics::Transform get(const std::string& from_frame, const std::string& to_frame)
        {
            ds.check_in(__PRETTY_FUNCTION__);
            if (from_frame == to_frame) return kinematics::identity(from_frame);
            try
            {
                ds.check_out();
                return ds.get<kinematics::Transform>(make_transform_name(from_frame, to_frame));
            }
            catch (const DataSourceException& )
            {
                CompositeTransformComputer computer(&ds, std::string("composite(")+make_transform_name(from_frame, to_frame)+")");
                computer.transforms = tree.get_path(from_frame, to_frame);
                ds.add(computer);
                tree.add(from_frame, to_frame); // Register compose transform in kinematic tree so it can be used when computing shortest paths
                add_inverse_module(from_frame, to_frame);
                const auto T = ds.get<kinematics::Transform>(make_transform_name(from_frame, to_frame));
                ds.check_out();
                return T;
            }
        }

    private:
        DataSource ds;
        KinematicTree tree;

};

void Kinematics::add(const Transform& t)
{
    pimpl->add(t);
}

Transform Kinematics::get(const std::string& from_frame, const std::string& to_frame)
{
    return pimpl->get(from_frame, to_frame);
}

Kinematics::Kinematics() : pimpl(new Impl())
{

}
