#ifndef DEFS_H
#define DEFS_H
/*! concatenating multiple args into one*/
#define CONCATE(...) __VA_ARGS__

/*! getter and setter generator for class memeber */
#define ADD_CLASS_FIELD(type, name, getter, setter) \
    public: \
        type& getter() { return m_##name; } \
        void setter(type name) { m_##name = name; } \
    private: \
        type m_##name;

#define ADD_CLASS_FIELD_NOSETTER(type, name, getter) \
    public: \
        type& getter() { return m_##name; } \
    private: \
        type m_##name;

#define ADD_CLASS_FIELD_PRIVATE(type, name ) \
    private: \
        type m_##name;
#endif // DEFS_H
