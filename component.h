#ifndef INCLUDED_PE32_COMPONENT_H
#define INCLUDED_PE32_COMPONENT_H

class Component {
public:
    virtual ~Component() {};
    virtual void setup() = 0;
    virtual void tick() = 0;
};

// vim: set ts=8 sw=4 sts=4 et ai:
#endif //INCLUDED_PE32_COMPONENT_H
