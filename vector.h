struct Vector2 {
    float x;
    float z;
    void add(const Vector2 &other) {
        x += other.x;
        z += other.z;
    }
    void scale(float scalar) {
        x *= scalar;
        z *= scalar;
    }
    float sqrMagnitude() { return x * x + z * z; }
};
