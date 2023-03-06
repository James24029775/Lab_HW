import React, {Component} from 'react';
import {withRouter} from 'react-router-dom';
import {connect} from 'react-redux';
import Nav from './Nav';
import Logo from "../../assets/images/login_page_logo.svg";


class SideBar extends Component {

  state = {};

  render() {
    return (
      <div className="sidebar">

        <div className="brand">
          <a href="/" className="brand-name">
            <img src={Logo} alt="logo" className="logo"/>
          </a>

        </div>

        <div className="sidebar-wrapper">
          {/*<UserInfo/>*/}
          {/*<div className="line"/>*/}
          <Nav/>
        </div>
      </div>
    )
  }
}

const mapStateToProps = state => ({});

export default withRouter(connect(mapStateToProps)(SideBar));
