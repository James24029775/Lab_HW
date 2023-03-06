import Serializable from "./Serializable";

// export default class Tenant extends Serializable{
//   id = '';
//   name = "";
//   coor = "";


//   constructor(id, name, coor) {
//     super();
//     this.id = id;
//     this.name = name;
//     this.coor = coor;
//   }
// }
export default class Mission extends Serializable{
  missionId = '';
  missionName = "";
  missionCoordinate = "";
  


  constructor(missionId, missionName, missionCoordinate) {
    super();
    this.missionId = missionId;
    this.missionName = missionName;
    this.missionCoordinate = missionCoordinate;
  }
}
