#pragma once

namespace relpos {
    enum Relations {
        NoRel,
        RightOf,
        LeftOf,
        Behind,
        FrontOf,
        Behind,
        FrontOf,
        Above,
        Below
    };

    struct Rel {
        Relations Rel;
        float Scale;
        float Space;
        float XOffset;
        float YOffset;
    };

    struct Pos {

    };
    
} // namespace relpos
