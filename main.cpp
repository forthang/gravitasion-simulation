#include "GravitySimulation.hpp" 
#include <iostream>              

int main() {
    GravitySimulation sim(1280, 720, "Gravity Simulation OOP");
    
    if (!sim.running) { 
        std::cerr << "GravitySimulation initialization failed. Exiting." << std::endl;
        return -1;
    }
    
    try {
        sim.run();
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred during simulation: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "An unknown exception occurred during simulation." << std::endl;
        return -1;
    }

    return 0;
}