#ifndef TOWER_PHYSICS_2D_H
#define TOWER_PHYSICS_2D_H

// Standalone planar rigid-body solver, independent of the PSP SDK and LTE.
// X = horizontal, Y = up, angular velocity/angle about Z.
// Deliberately small, fixed-capacity implementation for the first vertical slice.
namespace tower {

struct Vec2 {
    float x, y;
    Vec2(float xx=0.0f, float yy=0.0f) : x(xx), y(yy) {}
    Vec2 operator+(const Vec2& b) const { return Vec2(x+b.x,y+b.y); }
    Vec2 operator-(const Vec2& b) const { return Vec2(x-b.x,y-b.y); }
    Vec2 operator*(float a) const { return Vec2(x*a,y*a); }
    Vec2 operator/(float a) const { return Vec2(x/a,y/a); }
    Vec2& operator+=(const Vec2& b) { x+=b.x; y+=b.y; return *this; }
    Vec2& operator-=(const Vec2& b) { x-=b.x; y-=b.y; return *this; }
};

float dot(const Vec2& a, const Vec2& b);
float cross(const Vec2& a, const Vec2& b);

struct Body {
    Vec2 position;
    Vec2 velocity;
    float angle;            // radians (counterclockwise)
    float angularVelocity;  // radians / second
    float halfWidth, halfHeight;
    float invMass, invInertia;
    float friction, restitution;
    Body();
    static Body box(float width, float height, float mass, float x, float y);
    bool isDynamic() const { return invMass > 0.0f; }
};

class World {
public:
    enum { MAX_BODIES = 8, MAX_MANIFOLDS = 24, SOLVER_ITERATIONS = 18 };
    World();
    void reset();
    int addBody(const Body& body); // -1 if capacity reached
    Body& body(int index) { return bodies_[index]; }
    const Body& body(int index) const { return bodies_[index]; }
    int bodyCount() const { return count_; }
    int contactCount() const { return contacts_; } // count of contact points last step
    // Whether a body touched anything in the latest step, including ground.
    bool hasContact(int bodyIndex) const;
    int pairCount() const { return pairs_; }
    // True when the last physics step reported a support normal pointing
    // from lower to upper. Ground contact alone is not a successful stack.
    bool isSupportedBy(int upperBody, int lowerBody) const;
    // Live tuning takes effect on the next fixed physics step.
    void setDynamics(float gravity, float linearDamping, float angularDamping);
    void step(float dt); // must be called with dt = 1/60 for the prototype

private:
    struct Contact {
        int ia, ib, count;
        Vec2 normal;
        Vec2 points[2];
        float penetration;
        float normalImpulse[2];
        float tangentImpulse[2];
        float bounce[2];
        Contact();
    };
    Body bodies_[MAX_BODIES];
    Contact manifolds_[MAX_MANIFOLDS];
    int count_, contacts_, pairs_, manifoldCount_;
    float gravity_, linearDamping_, angularDamping_;
    bool collide(int ia, int ib, Contact& contact) const;
    void solve(Contact& contact);
};

} // namespace tower
#endif
