/*
 * Copyright (c) 2010, Maxim Likhachev
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Pennsylvania nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* \author Benjamin Cohen */

#ifndef _OCCUPANCY_GRID_
#define _OCCUPANCY_GRID_

#include <sys/stat.h>
#include <vector>
#include <fstream>
#include <tf/LinearMath/Vector3.h>
#include <Eigen/Geometry>
#include <distance_field/voxel_grid.h>
#include <distance_field/propagation_distance_field.h>
#include <arm_navigation_msgs/CollisionMap.h>


/* \brief At this point, this is a very lightweight layer on top of the
 * PropagationDistanceField class. I'll eventually get rid of it once the
 * PDF class has a couple of more things in it.
*/

namespace sbpl_arm_planner{

class OccupancyGrid
{
  public:
   
    /** 
     * @brief Constructor 
     * @param dimension of grid along X
     * @param dimension of grid along Y
     * @param dimension of grid along Z
     * @param resolution of grid (meters)
     * @param X coordinate of origin (meters)
     * @param Y coordinate of origin (meters)
     * @param Z coordinate of origin (meters)
    */
    OccupancyGrid(double dim_x, double dim_y, double dim_z, double resolution, double origin_x, double origin_y, double origin_z);

    /** @brief destructor */
    ~OccupancyGrid();

    /** @brief convert grid cell coords into world coords*/
    inline void gridToWorld(int x, int y, int z, double &wx, double &wy, double &wz);
    
    /** @brief convert world coords into grid cell coords*/
    inline void worldToGrid(double wx, double wy, double wz, int &x, int &y, int &z); 

    /** @brief get the cell's distance to the nearest obstacle in cells*/
    inline unsigned char getCell(int x, int y, int z);

    /** @brief get the cell's distance to the nearest obstacle in meters*/
    inline double getCell(int *xyz);
    
    inline double getDistance(int x, int y, int z);

    /** @brief check if {x,y,z} is in bounds of the grid */
    inline bool isInBounds(int x, int y, int z);

    /** @brief return a pointer to the distance field */
    inline distance_field::PropagationDistanceField* getDistanceFieldPtr();
    
    /** @brief get the dimensions of the grid */
    void getGridSize(int &dim_x, int &dim_y, int &dim_z);

    /** @brief get the dimensions of the grid */
    void getGridSize(int *dims); //FILL IN THIS FUNCTION

    /** @brief get the dimensions of the world (meters)*/
    void getWorldSize(double &dim_x, double &dim_y, double &dim_z);

    /** @brief get the origin of the world (meters)*/
    void getOrigin(double &wx, double &wy, double &wz);

    /** @brief get the resolution of the world (meters)*/
    double getResolution();

    /** @brief update the distance field from the collision_map */
    void updateFromCollisionMap(const arm_navigation_msgs::CollisionMap &collision_map);
    
    /** @brief display distance field visualizations to rviz */
//    void visualize();
    
    /** 
     * @brief manually add a cuboid to the collision map
     * @param X_origin_of_cuboid 
     * @param Y_origin_of_cuboid 
     * @param Z_origin_of_cuboid
     * @param size along the X dimension (meters)
     * @param size along the Y dimension (meters)
     * @param size along the Z dimension (meters)
    */
    void addCollisionCuboid(double origin_x, double origin_y, double origin_z, double size_x, double size_y, double size_z);

    void addPointsToField(const std::vector<Eigen::Vector3d> &points);

    void getVoxelsInBox(const geometry_msgs::Pose &pose, const std::vector<double> &dim, std::vector<Eigen::Vector3d> &voxels);

    std::string getReferenceFrame();

    void setReferenceFrame(const std::string &frame);

    void reset();

  private:

    double grid_resolution_;
    double prop_distance_;

    std::string reference_frame_;
    distance_field::PropagationDistanceField* grid_;
};

inline distance_field::PropagationDistanceField* OccupancyGrid::getDistanceFieldPtr()
{
  return grid_;
}

inline void OccupancyGrid::gridToWorld(int x, int y, int z, double &wx, double &wy, double &wz)
{
  grid_->gridToWorld(x, y, z, wx, wy, wz); 
}

inline void OccupancyGrid::worldToGrid(double wx, double wy, double wz, int &x, int &y, int &z)
{
  grid_->worldToGrid (wx, wy, wz, x, y, z);
}

inline double OccupancyGrid::getDistance(int x, int y, int z)
{
  return grid_->getDistanceFromCell(x,y,z);
}

inline unsigned char OccupancyGrid::getCell(int x, int y, int z)
{
  return (unsigned char)(grid_->getDistanceFromCell(x,y,z) / grid_resolution_);
}

inline double OccupancyGrid::getCell(int *xyz)
{
  return grid_->getDistanceFromCell(xyz[0],xyz[1],xyz[2]);
}

inline bool OccupancyGrid::isInBounds(int x, int y, int z)
{
  return (
      x>=0 && x<grid_->getNumCells(distance_field::PropagationDistanceField::DIM_X) &&
      y>=0 && y<grid_->getNumCells(distance_field::PropagationDistanceField::DIM_Y) &&
      z>=0 && z<grid_->getNumCells(distance_field::PropagationDistanceField::DIM_Z));
}

inline std::string OccupancyGrid::getReferenceFrame()
{
  return reference_frame_;
}

inline void OccupancyGrid::setReferenceFrame(const std::string &frame)
{
  reference_frame_ = frame;
}

inline void OccupancyGrid::addPointsToField(const std::vector<Eigen::Vector3d> &points)
{
  std::vector<tf::Vector3> pts(points.size());
  for(size_t i = 0; i < points.size(); ++i)
    pts[i] = tf::Vector3(points[i].x(), points[i].y(), points[i].z());
  
  grid_->addPointsToField(pts);
}


}

#endif
