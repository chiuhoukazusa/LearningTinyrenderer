#pragma once
#include "geometry.h"
#include "transform.h"
#include "shader/light/light.h"

//static auto eye_pos   = myEigen::Vector3f(0, 0, 2.5);
static auto eye_pos = myEigen::Vector3f(0, 0.8, 2.5);
static auto gaze_dir = myEigen::Vector3f(0, 0, -1);
static auto view_up = myEigen::Vector3f(0, 1, 0);
static float theta = 0;
static int width = 700;
static int height = 700;

static float zneardis = 0.1f;
static float zfardis = 50;
static float fovY = 45;
static float aspect = 1;

static rst::PointLight light = rst::PointLight(myEigen::Vector3f(20, 20, 20), 1400.0);
