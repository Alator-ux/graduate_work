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
    /// ph — photon map,
    /// rt - ray tracer
    /// </summary>
    enum class PathResponsible { pm, rt, none };
    PathOperator() {
        diffuse_surfs = 0;
        lastPathType = PathType::none;
    }
    void inform(PathType pt) {
        switch (pt) {
        case PathType::dif_refl:
            diffuse_surfs++;
            break;
        default:
            break;
        }
        lastPathType = pt;
    }
    PathResponsible response() {
        if (lastPathType == PathType::eye && diffuse_surfs < 2) {
            return PathResponsible::rt;
        } 
        if (lastPathType == PathType::dif_refl) {
            return PathResponsible::pm;
        }
        return PathResponsible::none;
    }
    void clear() {
        diffuse_surfs = 0;
        lastPathType = PathType::none;
    }
};

