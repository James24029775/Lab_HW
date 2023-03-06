
export default class missionActions {
  static SET_MISSIONS = 'MISSION/SET_MISSIONS';

  static setMissions(setmissions) {
    return {
      type: this.SET_MISSIONS,
      missions: setmissions, // send to "reducer>mission"
    };
  }
}
