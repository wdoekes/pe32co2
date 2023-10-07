#ifndef INCLUDED_PE32_SENSORVALUE_H
#define INCLUDED_PE32_SENSORVALUE_H

#include <Arduino.h>
#include <TrivialKalmanFilter.h>


/**
 * NumericGetterSetter Interface for getting/setting primitives
 */
template <typename T>
class NumericGetterSetter {
public:
    NumericGetterSetter() {}
    virtual ~NumericGetterSetter() {}
    virtual NumericGetterSetter& operator=(T new_value) = 0;
    virtual operator T() const = 0;

private:
    // Private copy constructor/assignments to restrict copying
    NumericGetterSetter(const NumericGetterSetter&) = delete;
    NumericGetterSetter& operator=(const NumericGetterSetter&) = delete;
};


/**
 * SensorValue implementation that implements NumericGetterSetter for easy
 * getting/setting and stores the value using a Kalman Filter.
 *
 * Additionally it keeps track of recent assignments, so we can tell if a
 * value is fresh.
 */
template <typename T>
class SensorValue : public NumericGetterSetter<T> {
public:
    // I have no idea at the moment how to get sane covariances for the
    // TrivialKalmanFilter constructor. Leaving them as the example values
    // right now.
    SensorValue(T initial = 0.0) : m_value(4.7e-3, 1e-5) {
        m_value.update(initial);
    }

    // Implementing the assignment operator from the primitive T (setter)
    SensorValue& operator=(T new_value) {
        m_value.update(new_value);
        m_last_updated = millis();
        if (m_last_updated == 0) {
            m_last_updated = 1;
        }
        return *this;
    }

    // Implementing the conversion operator to the primitive T (getter)
    operator T() const {
        return m_value.get();
    }

    // Has the value been updated in the last n seconds?
    bool is_fresh(unsigned seconds) const {
        if (m_last_updated == 0) {
            return false;
        }
        return ((millis() - m_last_updated) / 1000) < seconds;
    }

private:
    TrivialKalmanFilter<T> m_value;
    unsigned long m_last_updated;
};


/**
 * floatlike Interface
 *
 * Usage:
 *
 *   void readfunc(const floatlike& f) {
 *     printf("[%.1f]\n", (float)f);
 *   }
 *   void writefunc(floatlike& f) {
 *     f = 1.23f;
 *   }
 */
typedef NumericGetterSetter<float> floatlike;


/**
 * SensorFloat, implements floatlike
 *
 * Usage:
 *
 *   (everything that floatlike does)
 *   + more
 */
typedef SensorValue<float> SensorFloat;

// vim: set ts=8 sw=4 sts=4 et ai:
#endif //INCLUDED_PE32_SENSORVALUE_H
