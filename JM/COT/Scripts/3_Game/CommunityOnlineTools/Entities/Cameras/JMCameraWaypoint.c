// ============================================================
//  JMCameraWaypoint
//  Defined in 3_Game so both JMCinematicCamera (3_Game) and
//  JMCameraSerialize / JMCameraForm (5_Mission) can reference it.
// ============================================================

enum JMCameraEasing
{
	LINEAR        = 0,
	EASE_IN       = 1,   // accelerate from rest
	EASE_OUT      = 2,   // decelerate to rest
	EASE_IN_OUT   = 3,   // SmoothStep — accelerate then decelerate (default)
	SMOOTHER_STEP = 4,   // quintic — second-derivative continuous
	BOUNCE        = 4    // not yet used; reserved
}

class JMCameraWaypoint
{
	vector Position;
	float  Time              = 5.0;  // legacy: kept for JSON compat and fallback
	float  Speed             = 5.0;  // travel speed in m/s (0 = use Time fallback)
	bool   Smooth;            // legacy: kept for JSON compat
	vector Orientation;
	bool   OrientationCaptured;

	// Per-waypoint screen effects (interpolated while traveling TO this waypoint)
	float  Exposure          = 0.0;   // EV value
	float  Vignette          = 0.0;   // vignette intensity 0..1
	float  Blur              = 1.0;   // DOF blur 0..100
	float  FOV               = 1.0;   // camera FOV

	// Per-waypoint camera shake
	float  ShakeIntensity    = 0.0;
	float  ShakeFrequency    = 1.0;

	// Interpolation / easing
	bool           UseCatmull   = false;              // true = catmull-rom spline
	JMCameraEasing m_Easing       = JMCameraEasing.EASE_IN_OUT;

	// Hold at this waypoint before continuing (seconds)
	float  HoldTime          = 0.0;

	// Track the currently locked target during this segment instead of using captured orientation
	bool   TrackTarget       = false;
}
