// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#ifndef MOAISURFACE2D_H
#define MOAISURFACE2D_H

class MOAISurface2D;

//================================================================//
// MOAISurfaceEdge2D
//================================================================//
class MOAISurfaceEdge2D {
public:

	USVec2D		mV0;
	USVec2D		mV1;
};

//================================================================//
// USSurfaceTouch2D
//================================================================//
//class USSurfaceTouch2D {
//public:
//
//	// input
//	USVec2D		mFinger; // touch vector (unit vector)
//
//	// internal
//	float		mDist;
//	bool		mTouch;
//	bool		mPrevTouch;
//	bool		mHit;
//
//	// output
//	USVec2D		mPoint; // touch point
//	USVec2D		mNorm;
//	
//	//----------------------------------------------------------------//
//	void		Init			( USVec2D finger );
//	void		Reset			( float dist = 1.25f );
//	void		Scale			( float xSc, float ySc );
//};

//================================================================//
// MOAISurfaceHit2D
//================================================================//
class MOAISurfaceHit2D {
public:

	float		mTime;
	USVec2D		mPoint;
	USVec2D		mNorm;
};

//================================================================//
// MOAISurface2D
//================================================================//
class MOAISurface2D :
	public USPlane2D,
	public MOAISurfaceEdge2D {
public:

	USVec2D			mTangent; // perp to norm
	
	float			mP0;
	float			mP1;
	
	float			mXMin;
	float			mXMax;
	
	//----------------------------------------------------------------//
	void			ClampPoint			( USVec2D& p ) const;
	bool			GetContact			( USVec2D& sphereLoc, USVec2D& contact, USVec2D& norm ) const;
	float			GetDepthAlongRay	( USVec2D& sphereLoc, USVec2D& ray ) const;
	bool			GetHit				( USVec2D& sphereLoc, USVec2D& move, MOAISurfaceHit2D& hit ) const; // unit sphere
	bool			GetRayHit			( USVec2D& loc, USVec2D& ray, float& time ) const; // ray, no edges
	bool			GetRayHit			( USVec2D& loc, USVec2D& ray, float pad, float& time ) const; // ray
	//void			GetSnapUp			( USVec2D& loc, float maxSnap, USVec2D& move, SurfaceSnap2D& snap );
	//bool			GetTouch			( USVec2D& sphereLoc, USSurfaceTouch2D& touch ); // unit sphere
	void			Init				( const USVec2D& v0, const USVec2D& v1 );
	bool			IsBridge			( USVec2D& loc, USVec2D& move ) const;
	bool			IsLeaving			( USVec2D& loc, USVec2D& move ) const;
	bool			IsOn				( USVec2D& loc ) const;
	bool			IsOver				( USVec2D& loc ) const;
};

#endif
