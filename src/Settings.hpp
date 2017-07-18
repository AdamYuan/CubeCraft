#pragma once

#define MOVE_DIST 0.5f
#define MOUSE_SENSITIVITY 0.17f

#define THREAD_NUM 3
#define CHUNK_SIZE 32
#define CHUNK_LOAD_DISTANCE 8 //chunks
#define VIEW_DISTANCE ((CHUNK_LOAD_DISTANCE - 1) * CHUNK_SIZE) //blocks
#define GRAVITY 3.0f //gravity
#define JUMP_STEP 0.85f
#define PHYSICS true
#define HITTEST_DELTA 0.001f
#define MAX_MOVE_DIST 16.0f
#define TJUNC_DELTA 0.001f //dealing with t-junction
