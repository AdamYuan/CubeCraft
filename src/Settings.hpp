#pragma once

#define MOVE_DIST 0.4f
#define MOUSE_SENSITIVITY 0.17f

#define THREAD_NUM 3
#define MAX_UPDATE_THREAD_IN_FRAME 5
#define MAX_LOADING_THREAD_IN_FRAME 5
#define CHUNK_SIZE 32
#define CHUNK_LOAD_DISTANCE 8 //chunks
#define VIEW_DISTANCE ((CHUNK_LOAD_DISTANCE - 1) * CHUNK_SIZE) //blocks
#define GRAVITY 3.0f //gravity
#define JUMP_STEP 0.85f
#define PHYSICS true
#define HITTEST_DELTA 0.001f
#define MAX_MOVE_DIST 16.0f
