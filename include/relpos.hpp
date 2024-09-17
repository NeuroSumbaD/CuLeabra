#pragma once
#include <string>
#include "math.hpp"

namespace relpos {
    enum Relations {
        NoRel,
        RightOf,
        LeftOf,
        Behind,
        FrontOf,
        Above,
        Below
    };

    enum XAligns {
        Left,
        Middle,
        Right
    };

    enum YAligns {
        Front,
        Center,
        Back
    };

    struct Rel {
        Relations Rel;
        float Scale;
        float Space;
        float XOffset;
        float YOffset;
    };

    struct Pos {
        // spatial relationship between this layer and the other layer
        Relations Rel;

        // ] horizontal (x-axis) alignment relative to other
        XAligns XAlign;

        // ] vertical (y-axis) alignment relative to other
        YAligns YAlign;

        // name of the other layer we are in relationship to
        std::string Other;

        // scaling factor applied to layer size for displaying
        float Scale = 0;

        // number of unit-spaces between us
        float Space = 0;

        // for vertical (y-axis) alignment, amount we are offset relative to perfect alignment
        float XOffset = 0;

        // for horizontial (x-axis) alignment, amount we are offset relative to perfect alignment
        float YOffset = 0;

        // Pos is the computed position of lower-left-hand corner of layer
        // in 3D space, computed from the relation to other layer.
        math::Vector3 Pos;

        void Defaults();
        bool ShouldDisplay(std::string field);
        void SetRightOf(std::string other, float space);
        void SetBehind(std::string other, float space);
        void SetAbove(std::string other);
        void SetPos(math::Vector3 op, math::Vector2 osz, math::Vector2 sz);
        float AlignYPos(float yop, float yosz, float ysz);
        float AlignXPos(float xop, float xosz, float xsz);
    };
    
} // namespace relpos
