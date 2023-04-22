#pragma once
#include "Photon.h"
#include "Tools.h"
/* Photon Book page 85
Is this simple visualization a full solution to the rendering equation? To
answer this question we can look at the paths traced by the photons and
the rays and see if they cover the space of all paths.
L(S|D)*D are all the paths represented by the photon map.
(LS*E)|(DS*E) are all the paths traced by the ray tracer.*/
/* o - empty node
*      
*      ^ -> s
*      |    |
*      |    v
* L -> o <- o
*      |    ^
*      |    |
*      v -> d -> pm
* 
* 
*      ^ -> d
*      |    | <- ^
*      |    v    |
* L -> o -> o -> s
*           |
*           v -> e -> rt
* 
*/
class PathOperator
{
    int diffuse_surfs;
    PathType lastPathType;
public:
    /// <summary>
    /// ph Ч photon map,
    /// rt - ray tracer
    /// </summary>
    PathOperator() {
        clear();
    }
    void inform(PathType pt) {
        if (pt == PathType::dif_refl) {
            diffuse_surfs++;
        }
        lastPathType = pt;
    }
    /// <summary>
    /// Returns true if caustic map needs to be filled in
    /// </summary>
    bool response() {
        if ((lastPathType == PathType::spec_refl) && diffuse_surfs < 2) {
            return true;
        } 
        if (lastPathType == PathType::dif_refl) {
            return false;
        }
        return false; // мб заменить на енамы и другой ретерн сделать
        //return ;
    }
    void clear() {
        diffuse_surfs = 0;
        lastPathType = PathType::none;
    }
};

