/*
 * Seabed.cpp
 */

#include "Seabed.hpp"
#include "xdyn/core/AssetPath.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stb_image.h>

Seabed::Seabed(const std::string sea_bed_file)
    : seabed(), width(), height(), xscale(1), yscale(1), zscale(1), xoffset(0), yoffset(0)
{
    const std::string seabed_path = xdyn::resolve_asset(sea_bed_file);
    int channels;
    unsigned char* img = stbi_load(seabed_path.c_str(), &width, &height, &channels, 1);
    if (!img)
    {
        std::cerr << "Failed to load seabed image " << seabed_path << ": " << stbi_failure_reason() << std::endl;
    }
    for (int i = 0; i < width * height; i++)
    {
        int x = i % width;
        int y = i / width;
        seabed[{x, y}] = img[i];
    }
    stbi_image_free(img);
}

Seabed::Seabed(const double depth)
    : seabed(), width(1), height(1), xscale(1), yscale(1), zscale(1), xoffset(0), yoffset(0)
{
    seabed[{0, 0}] = depth;
}

double Seabed::get_height(const double x, const double y) const
{
    if (width == 1 && height == 1)
    {
        return seabed.at({0, 0});
    }
    const InterpolationWeights weights = calculateWeights(x, y);
    const double h_ul = seabed.at({weights.xleft, weights.yup});
    const double h_ur = seabed.at({weights.xright, weights.yup});
    const double h_dl = seabed.at({weights.xleft, weights.ydown});
    const double h_dr = seabed.at({weights.xright, weights.ydown});
    const double interpolated = weights.left * weights.up * h_ul +
                                weights.right * weights.up * h_ur +
                                weights.left * weights.down * h_dl +
                                weights.right * weights.down * h_dr;
    return zscale * interpolated;
}

Eigen::Vector2d Seabed::get_gradient(const double x, const double y) const
{
    const double x_rel = x / xscale - xoffset;
    const double y_rel = y / yscale - yoffset;
    if (x_rel < 0 || x_rel > width || y_rel < 0 || y_rel > height)
    {
        return Eigen::Vector2d::Zero();
    }
    const InterpolationWeights weights = calculateWeights(x, y);
    const double h_dr = seabed.at({weights.xright, weights.ydown});
    const double h_dl = seabed.at({weights.xleft, weights.ydown});
    const double h_ur = seabed.at({weights.xright, weights.yup});
    const double h_ul = seabed.at({weights.xleft, weights.yup});
    const double gx = weights.down * (h_dr - h_dl) + weights.up * (h_ur - h_ul);
    const double gy = weights.left * (h_ul - h_dl) + weights.right * (h_ur - h_dr);
    return zscale * Eigen::Vector2d{gx, gy};
}

InterpolationWeights Seabed::calculateWeights(const double x_abs, const double y_abs) const
{
    const double x = x_abs / xscale - xoffset;
    const double y = y_abs / yscale - yoffset;
    const int xleft = std::max(0, std::min(width - 1, (int)std::floor(x)));
    const int xright = std::max(0, std::min(width - 1, (int)std::ceil(x)));
    const int ydown = std::max(0, std::min(height - 1, (int)std::floor(y)));
    const int yup = std::max(0, std::min(height - 1, (int)std::ceil(y)));
    double left, right, down, up;
    if (x <= 0)
    {
        left = 1;
        right = 0;
    }
    else if (x >= width - 1)
    {
        left = 0;
        right = 1;
    }
    else
    {
        left = x - xleft;
        right = xright - x;
    }
    if (y <= 0)
    {
        up = 0;
        down = 1;
    }
    else if (y >= height - 1)
    {
        up = 1;
        down = 0;
    }
    else
    {
        up = yup - y;
        down = y - ydown;
    }
    return {left, right, down, up, xleft, xright, ydown, yup};
}
