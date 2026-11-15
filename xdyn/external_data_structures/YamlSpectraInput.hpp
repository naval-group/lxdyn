#ifndef YAMLSPECTRAINPUT_HPP_
#define YAMLSPECTRAINPUT_HPP_

struct YamlAiry
{
    int seed{0}; // Random generator seed (to generate the random phases)
};

struct YamlDiracDirectionalSpreading
{
    double psi0{0}; //!< Primary wave direction in radians.
};

struct YamlDiracSpectralDensity
{
    double omega0{0}; //!< Angular frequency (\f$2\pi f\f$) in rad/s of the significant wave height
    double Hs{0};     //!< Significant wave height (in meters)
};

struct YamlPiersonMoskowitzSpectralDensity
{
    double Hs{0};   //!< Significant wave height (in meters)
    double Tp{0};   //!< Mean wave period (in seconds));
};

#endif /* YAMLSPECTRAINPUT_HPP_ */
