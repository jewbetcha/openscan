#pragma once

#include <Arduino.h>

void pulser_begin();

// Fires a single trigger pulse. SAFETY: caller is responsible for ensuring
// the laser is pointed somewhere safe (beam dump, far wall) and goggles are
// worn during bench testing.
void pulser_fire();
