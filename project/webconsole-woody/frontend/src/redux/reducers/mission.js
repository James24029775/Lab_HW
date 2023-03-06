import actions from '../actions/missionActions';

const initialState = {
  missions: [],
  missionsMap: {}
};

export default function reducer(state = initialState, action) {
  let nextState = {...state};

  switch (action.type) {
    case actions.SET_MISSIONS:
      nextState.missions = action.missions;
      nextState.missionsMap = createmissionsMap(action.missions);
      return nextState;

    default:
      return state;
  }
}

function createmissionsMap(action_missions) {
  let missionsMap = {};
  action_missions.forEach(missions => missionsMap[missions['missionId']] = action_missions);
  return missionsMap;
}
