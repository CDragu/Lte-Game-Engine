#include "Physics2D.h"
#include <math.h>

namespace tower {
static float absf(float v) { return v < 0.0f ? -v : v; }
static float maxf(float a,float b) { return a>b?a:b; }
static float minf(float a,float b) { return a<b?a:b; }
static float clampf(float x,float lo,float hi) { return maxf(lo,minf(x,hi)); }

float dot(const Vec2& a, const Vec2& b) { return a.x*b.x + a.y*b.y; }
float cross(const Vec2& a, const Vec2& b) { return a.x*b.y - a.y*b.x; }
static Vec2 crossSV(float s, const Vec2& v) { return Vec2(-s*v.y,s*v.x); }

Body::Body()
 : position(), velocity(), angle(0.0f), angularVelocity(0.0f),
   halfWidth(0.5f), halfHeight(0.5f), invMass(0.0f), invInertia(0.0f),
   friction(0.78f), restitution(0.03f) {}

Body Body::box(float width, float height, float mass, float x, float y)
{
    Body body;
    body.position=Vec2(x,y);
    body.halfWidth=width*0.5f;
    body.halfHeight=height*0.5f;
    if (mass>0.0f) {
        body.invMass=1.0f/mass;
        // Inertia of a uniform planar rectangle around the Z axis.
        body.invInertia=12.0f/(mass*(width*width+height*height));
    }
    return body;
}

World::Contact::Contact() : ia(-1),ib(-1),count(0),normal(),penetration(0.0f)
{
    normalImpulse[0]=normalImpulse[1]=0.0f;
    tangentImpulse[0]=tangentImpulse[1]=0.0f;
    bounce[0]=bounce[1]=0.0f;
}

World::World() : gravity_(12.0f), linearDamping_(0.04f), angularDamping_(0.06f), windAccelerationX_(0.0f) { reset(); }
void World::reset() { count_=contacts_=manifoldCount_=0; windAccelerationX_=0.0f; }

void World::setWindAcceleration(float x)
{
    if(x>=-6.0f && x<=6.0f) windAccelerationX_=x;
}

void World::setDynamics(float gravity,float linearDamping,float angularDamping)
{
    if(gravity>=0.0f && gravity<=35.0f) gravity_=gravity;
    if(linearDamping>=0.0f && linearDamping<=3.0f) linearDamping_=linearDamping;
    if(angularDamping>=0.0f && angularDamping<=3.0f) angularDamping_=angularDamping;
}

int World::addBody(const Body& body)
{
    if(count_>=MAX_BODIES) return -1;
    bodies_[count_]=body;
    return count_++;
}

static Vec2 axisX(const Body& b) { return Vec2(cosf(b.angle),sinf(b.angle)); }
static Vec2 axisY(const Body& b) { return Vec2(-sinf(b.angle),cosf(b.angle)); }

static void corners(const Body& b, Vec2 out[4])
{
    Vec2 ux=axisX(b)*b.halfWidth, uy=axisY(b)*b.halfHeight;
    out[0]=b.position-ux-uy;
    out[1]=b.position+ux-uy;
    out[2]=b.position+ux+uy;
    out[3]=b.position-ux+uy;
}

static bool inside(const Vec2& p,const Body& b)
{
    const Vec2 d=p-b.position;
    return absf(dot(d,axisX(b)))<=b.halfWidth+0.0001f &&
           absf(dot(d,axisY(b)))<=b.halfHeight+0.0001f;
}

static bool intersectSegments(const Vec2& a,const Vec2& b,
                              const Vec2& c,const Vec2& d,Vec2& hit)
{
    const Vec2 r=b-a,s=d-c;
    const float den=cross(r,s);
    if(absf(den)<0.000001f) return false;
    const float t=cross(c-a,s)/den, u=cross(c-a,r)/den;
    if(t<-0.0001f || t>1.0001f || u<-0.0001f || u>1.0001f) return false;
    hit=a+r*clampf(t,0.0f,1.0f);
    return true;
}

bool World::collide(int ia,int ib,Contact& out) const
{
    const Body& a=bodies_[ia], &b=bodies_[ib];
    if(!a.isDynamic() && !b.isDynamic()) return false;
    const Vec2 axes[4]={axisX(a),axisY(a),axisX(b),axisY(b)};
    const Vec2 diff=b.position-a.position;
    float minDepth=1.0e20f;
    Vec2 normal;
    for(int k=0;k<4;k++) {
        const Vec2 n=axes[k];
        const float radiusA=a.halfWidth*absf(dot(n,axes[0]))+
                            a.halfHeight*absf(dot(n,axes[1]));
        const float radiusB=b.halfWidth*absf(dot(n,axes[2]))+
                            b.halfHeight*absf(dot(n,axes[3]));
        const float distance=dot(diff,n);
        const float overlap=radiusA+radiusB-absf(distance);
        if(overlap<=0.0f) return false;
        if(overlap<minDepth) {
            minDepth=overlap;
            normal=distance>=0.0f?n:n*(-1.0f);
        }
    }

    Vec2 pa[4],pb[4],possible[24];
    int nPossible=0;
    corners(a,pa); corners(b,pb);
    for(int i=0;i<4;i++) {
        if(inside(pa[i],b)) possible[nPossible++]=pa[i];
        if(inside(pb[i],a)) possible[nPossible++]=pb[i];
    }
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++) {
            Vec2 point;
            if(intersectSegments(pa[i],pa[(i+1)%4],pb[j],pb[(j+1)%4],point))
                possible[nPossible++]=point;
        }
    // Deeply enclosed boxes may have ambiguous features; use midpoint fallback.
    if(nPossible==0) possible[nPossible++]=(a.position+b.position)*0.5f;

    // Keep the extreme features along the tangent to resist rocking.
    const Vec2 tangent(-normal.y,normal.x);
    int lo=0,hi=0;
    for(int i=1;i<nPossible;i++) {
        if(dot(possible[i],tangent)<dot(possible[lo],tangent)) lo=i;
        if(dot(possible[i],tangent)>dot(possible[hi],tangent)) hi=i;
    }
    out=Contact();
    out.ia=ia; out.ib=ib; out.normal=normal; out.penetration=minDepth;
    out.points[0]=possible[lo];
    out.count=1;
    if((possible[hi]-possible[lo]).x*(possible[hi]-possible[lo]).x+
       (possible[hi]-possible[lo]).y*(possible[hi]-possible[lo]).y > 0.0004f) {
        out.points[1]=possible[hi];
        out.count=2;
    }
    for(int k=0;k<out.count;k++) {
        const Vec2 ra=out.points[k]-a.position, rb=out.points[k]-b.position;
        const Vec2 rv=(b.velocity+crossSV(b.angularVelocity,rb))-
                      (a.velocity+crossSV(a.angularVelocity,ra));
        const float vn=dot(rv,normal);
        // No restitution on quiet contacts: avoids towers bouncing at rest.
        out.bounce[k]=vn < -1.0f ? (-minf(a.restitution,b.restitution)*vn) : 0.0f;
    }
    return true;
}

static void impulse(Body& a,Body& b,const Vec2& dir,float j,
                    const Vec2& ra,const Vec2& rb)
{
    const Vec2 p=dir*j;
    if(a.isDynamic()) {
        a.velocity-=p*a.invMass;
        a.angularVelocity-=cross(ra,p)*a.invInertia;
    }
    if(b.isDynamic()) {
        b.velocity+=p*b.invMass;
        b.angularVelocity+=cross(rb,p)*b.invInertia;
    }
}

void World::solve(Contact& c)
{
    Body& a=bodies_[c.ia], &b=bodies_[c.ib];
    const Vec2 n=c.normal, t(-n.y,n.x);
    const float friction=sqrtf(maxf(0.0f,a.friction*b.friction));
    for(int k=0;k<c.count;k++) {
        const Vec2 ra=c.points[k]-a.position, rb=c.points[k]-b.position;
        Vec2 rv=(b.velocity+crossSV(b.angularVelocity,rb))-
                (a.velocity+crossSV(a.angularVelocity,ra));
        const float rna=cross(ra,n), rnb=cross(rb,n);
        const float denom=a.invMass+b.invMass+
                          rna*rna*a.invInertia+rnb*rnb*b.invInertia;
        if(denom<0.000001f) continue;

        // Sequential impulses, accumulated per contact for stable friction cones.
        const float oldN=c.normalImpulse[k];
        const float candidate=(c.bounce[k]-dot(rv,n))/denom;
        c.normalImpulse[k]=maxf(oldN+candidate,0.0f);
        impulse(a,b,n,c.normalImpulse[k]-oldN,ra,rb);

        rv=(b.velocity+crossSV(b.angularVelocity,rb))-
           (a.velocity+crossSV(a.angularVelocity,ra));
        const float rta=cross(ra,t), rtb=cross(rb,t);
        const float tangentDenom=a.invMass+b.invMass+
                                rta*rta*a.invInertia+rtb*rtb*b.invInertia;
        if(tangentDenom<0.000001f) continue;
        const float oldT=c.tangentImpulse[k];
        const float limit=friction*c.normalImpulse[k];
        c.tangentImpulse[k]=clampf(oldT-dot(rv,t)/tangentDenom,-limit,limit);
        impulse(a,b,t,c.tangentImpulse[k]-oldT,ra,rb);
    }
}

bool World::hasContact(int index) const
{
    if(index<0 || index>=count_) return false;
    for(int i=0;i<manifoldCount_;++i)
        if(manifolds_[i].ia==index || manifolds_[i].ib==index) return true;
    return false;
}

bool World::isSupportedBy(int upperBody, int lowerBody) const
{
    if (upperBody<0 || lowerBody<0 || upperBody>=count_ ||
        lowerBody>=count_ || upperBody==lowerBody) return false;
    for(int m=0;m<manifoldCount_;m++) {
        const Contact& c=manifolds_[m];
        if(c.ia==lowerBody && c.ib==upperBody && c.normal.y>0.55f)
            return true;
        if(c.ib==lowerBody && c.ia==upperBody && c.normal.y< -0.55f)
            return true;
    }
    return false;
}

void World::step(float dt)
{
    // Caller is responsible for a fixed time step; reject bad inputs.
    if(!(dt>0.0f && dt<=0.05f)) return;
    // Bounded implicit damping; rates are per second.
    const float linearRetention=1.0f/(1.0f+linearDamping_*dt);
    const float angularRetention=1.0f/(1.0f+angularDamping_*dt);
    for(int i=0;i<count_;i++) {
        Body& b=bodies_[i];
        if(!b.isDynamic()) continue;
        b.velocity.y-=gravity_*dt;
        // Wind pressure acts on the vertical face above its centre,
        // transferring both horizontal momentum and a rocking torque.
        // Larger faces catch more wind; heavier objects resist it.
        const float force=windAccelerationX_*(b.halfHeight*2.0f);
        b.velocity.x+=force*b.invMass*dt;
        b.angularVelocity-=force*(b.halfHeight*0.45f)*b.invInertia*dt;
        b.velocity=b.velocity*linearRetention;
        b.angularVelocity*=angularRetention;
        b.position+=b.velocity*dt;
        b.angle+=b.angularVelocity*dt;
    }

    contacts_=0;
    manifoldCount_=0;
    for(int i=0;i<count_;i++)
        for(int j=i+1;j<count_;j++) {
            if(!bodies_[i].isDynamic() && !bodies_[j].isDynamic()) continue;
            if(manifoldCount_>=MAX_MANIFOLDS) continue;
            Contact c;
            if(collide(i,j,c)) {
                manifolds_[manifoldCount_++]=c;
                contacts_+=c.count;
            }
        }

    for(int iteration=0;iteration<SOLVER_ITERATIONS;iteration++)
        for(int m=0;m<manifoldCount_;m++) solve(manifolds_[m]);

    // Baumgarte-style penetration correction; static boxes stay fixed.
    // This simple solver is intended for a handful of blocks, not a large tower.
    for(int m=0;m<manifoldCount_;m++) {
        Contact& c=manifolds_[m];
        Body& a=bodies_[c.ia], &b=bodies_[c.ib];
        const float mass=a.invMass+b.invMass;
        if(mass<=0.0f) continue;
        const float correction=0.45f*maxf(c.penetration-0.003f,0.0f)/mass;
        const Vec2 delta=c.normal*correction;
        if(a.isDynamic()) a.position-=delta*a.invMass;
        if(b.isDynamic()) b.position+=delta*b.invMass;
    }
}
} // namespace tower
