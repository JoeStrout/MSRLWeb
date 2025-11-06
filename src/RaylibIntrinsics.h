//
//  RaylibIntrinsics.h
//  MSRLWeb
//
//  Raylib intrinsics for MiniScript
//

#ifndef RAYLIBINTRINSICS_H
#define RAYLIBINTRINSICS_H

namespace MiniScript {
	class Interpreter;
}

// Add Raylib intrinsics to the given MiniScript interpreter
void AddRaylibIntrinsics(MiniScript::Interpreter* interpreter);

#endif // RAYLIBINTRINSICS_H
