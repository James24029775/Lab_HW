import React, {Component} from 'react';
import {withRouter} from 'react-router-dom';
import {connect} from 'react-redux';

class UserInfo extends Component {

  state = {
    isShowingUserMenu: false
  };

  render() {
    let {user} = this.props;
    return (
      <div className="user-wrapper">
        <div className="user">
          <img src={user.imageUrl} alt={user.name} className="photo"/>
          <div className="userinfo">
            <div className="username">
              {user.username}
            </div>
            <div className="title">{user.name}</div>
          </div>

        </div>
      </div>
    );
  }

  isPathActive(path) {
    return this.props.location.pathname.startsWith(path);
  }
}

const mapStateToProps = state => ({
  user: state.auth.user
});

export default withRouter(connect(mapStateToProps)(UserInfo));
