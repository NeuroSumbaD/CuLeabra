#include "relpos.hpp"

// Defaults sets default scale, space, offset values.
// The relationship and align must be set specifically.
// These are automatically applied if Scale = 0
void relpos::Pos::Defaults() {
    if (Scale == 0) {
		Scale = 1;
	}
	if (Space == 0) {
		Space = 5;
	}
}

bool relpos::Pos::ShouldDisplay(std::string field) {
	if (field == "XAlign"){
		return Rel == FrontOf || Rel == Behind || Rel == Above || Rel == Below;
    } else if (field == "YAlign"){
		return Rel == LeftOf || Rel == RightOf || Rel == Above || Rel == Below;
    } else {
		return true;
    }
}

// SetRightOf sets a RightOf relationship with default YAlign:
// Front alignment and given spacing.
void relpos::Pos::SetRightOf(std::string other, float space) {
    Rel = RightOf;
	Other = other;
	YAlign = Front;
	Space = space;
	Scale = 1;
}

// SetBehind sets a Behind relationship with default XAlign:
// Left alignment and given spacing.
void relpos::Pos::SetBehind(std::string other, float space) {
    Rel = Behind;
	Other = other;
	XAlign = Left;
	Space = space;
	Scale = 1;
}

// SetAbove returns an Above relationship with default XAlign:
// Left, YAlign: Front alignment
void relpos::Pos::SetAbove(std::string other) {
    Rel = Above;
	Other = other;
	XAlign = Left;
	YAlign = Front;
	YOffset = 1;
	Scale = 1;
}

// SetPos sets the relative position based on other layer
// position and size, using current settings.
// osz and sz must both have already been scaled by
// relevant Scale factor.
void relpos::Pos::SetPos(math::Vector3 op, math::Vector2 osz, math::Vector2 sz) {
    if (Scale == 0) {
		Defaults();
	}
	Pos = op;
	switch (Rel) {
        case NoRel:
            return;
            break;
        case RightOf:
            Pos.X = op.X + osz.X + Space;
            Pos.Y = AlignYPos(op.Y, osz.Y, sz.Y);
            break;
        case LeftOf:
            Pos.X = op.X - sz.X - Space;
            Pos.Y = AlignYPos(op.Y, osz.Y, sz.Y);
            break;
        case Behind:
            Pos.Y = op.Y + osz.Y + Space;
            Pos.X = AlignXPos(op.X, osz.X, sz.X);
            break;
        case FrontOf:
            Pos.Y = op.Y - sz.Y - Space;
            Pos.X = AlignXPos(op.X, osz.X, sz.X);
            break;
        case Above:
            Pos.Z += 1;
            Pos.X = AlignXPos(op.X, osz.X, sz.X);
            Pos.Y = AlignYPos(op.Y, osz.Y, sz.Y);
            break;
        case Below:
            Pos.Z -= 1;
            Pos.X = AlignXPos(op.X, osz.X, sz.X);
            Pos.Y = AlignYPos(op.Y, osz.Y, sz.Y);
            break;
	}    
}

// AlignYPos returns the Y-axis (within-plane vertical or height)
// position according to alignment factors.
float relpos::Pos::AlignYPos(float yop, float yosz, float ysz) {
    switch (YAlign) {
        case Front:
            return yop + YOffset;
            break;
        case Center:
            return yop + 0.5*yosz - 0.5*ysz + YOffset;
            break;
        case Back:
            return yop + yosz - ysz + YOffset;
            break;
	}
	return yop;
}

// AlignXPos returns the X-axis (within-plane horizontal or width)
// position according to alignment factors.
float relpos::Pos::AlignXPos(float xop, float xosz, float xsz) {
    switch (XAlign) {
        case Left:
            return xop + XOffset;
            break;
        case Middle:
            return xop + 0.5*xosz - 0.5*xsz + XOffset;
            break;
        case Right:
            return xop + xosz - xsz + XOffset;
            break;
	}
	return xop;
}
