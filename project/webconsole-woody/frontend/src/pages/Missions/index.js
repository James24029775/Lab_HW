import React from 'react';
import {Route} from 'react-router-dom';
import MissionOverview from "./MissionOverview";

const Missions = ({match}) => (
  <div className="content">
    <Route exact path={`${match.url}/`} component={MissionOverview} />
  </div>
);

export default Missions;
