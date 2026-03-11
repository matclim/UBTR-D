#pragma once
#include "G4UserEventAction.hh"

class TrackerRunAction;

class TrackerEventAction : public G4UserEventAction {
public:
    explicit TrackerEventAction(TrackerRunAction* runAction);
    ~TrackerEventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*)   override;

private:
    TrackerRunAction* fRunAction;
    int fTubeHCID = -1;
    int fTileHCID = -1;
};
