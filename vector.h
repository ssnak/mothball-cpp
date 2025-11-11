template <typename T>
struct Vector2 {
    T x;
    T z;
    void add(const Vector2 &other) {
        x += other.x;
        z += other.z;
    }
    void scale(T scalar) {
        x *= scalar;
        z *= scalar;
    }
    T sqrMagnitude() { return x * x + z * z; }
};
