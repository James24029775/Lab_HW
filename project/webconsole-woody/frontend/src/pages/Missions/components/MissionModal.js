import React, { Component } from 'react';
import { Modal } from "react-bootstrap";
import Form from "react-jsonschema-form";
import PropTypes from 'prop-types';

class MissionModal extends Component {
  static propTypes = {
    open: PropTypes.bool.isRequired,
    setOpen: PropTypes.func.isRequired,
    tenant: PropTypes.object,
    onModify: PropTypes.func.isRequired,
    onSubmit: PropTypes.func.isRequired,
  };

  state = {
    editMode: false,
    formData: undefined,
    // for force re-rendering json form
    rerenderCounter: 0,
  };

  schema = {
    // Modal will save the data in the schema as {properties : "content"}
    
    // title: "A registration form",
    // "description": "A simple form example.",
    type: "object",
    required: [
      "missionName",
    ],
    properties: {
      missionId: {
        type: "string",
        title: "Mission ID",
        pattern: "^[0-9a-zA-Z-]*$",
        default: "",
        readOnly: true,
      },
      missionName: {
        type: "string",
        title: "Mission Name",
        default: "",
      },
      missionCoordinate:{
        type: "string",
        title: "Mission Coordinate",
        default: "",
      }
    },
  };

  componentDidUpdate(prevProps, prevState, snapshot) {
    if (prevProps !== this.props) {
      this.setState({ editMode: !!this.props.mission });

      if (this.props.mission) {
        const missionData = this.props.mission;

        let formData = {
          missionId: missionData['missionId'],
          missionName: missionData['missionName'],
          missionCoordinate: missionData['missionCoordinate'],

        };

        this.updateFormData(formData).then();
      } else {
        let formData = {
          missionId: "",
          missionName: "",
          missionCoordinate: "",
        };
        this.updateFormData(formData).then();
      }
    }
  }

  async onChange(data) {
    const lastData = this.state.formData;

    if (lastData && lastData.missionId === undefined)
      lastData.missionId = "";
  }

  async updateFormData(newData) {
    // Workaround for bug: https://github.com/rjsf-team/react-jsonschema-form/issues/758
    await this.setState({ rerenderCounter: this.state.rerenderCounter + 1 });
    await this.setState({
      rerenderCounter: this.state.rerenderCounter + 1,
      formData: newData,
    });
  }

  onSubmitClick(result) {
    const formData = result.formData;

    let missionData = {
      "missionId": formData["missionId"],
      "missionName": formData["missionName"],
      "missionCoordinate": formData["missionCoordinate"],
    };

    if(this.state.editMode) {
      this.props.onModify(missionData);
    } else {
      this.props.onSubmit(missionData);
    }
  }

  render() {
    return (
      <Modal
        show={this.props.open}
        className={"fields__edit-modal theme-light"}
        backdrop={"static"}
        onHide={this.props.setOpen.bind(this, false)}>
        <Modal.Header closeButton>
          <Modal.Title id="example-modal-sizes-title-lg">
            {this.state.editMode ? "Edit Tenant" : "New Tenant"}
          </Modal.Title>
        </Modal.Header>

        <Modal.Body>
          {this.state.rerenderCounter % 2 === 0 &&
            <Form schema={this.schema}
              formData={this.state.formData}
              onChange={this.onChange.bind(this)}
              onSubmit={this.onSubmitClick.bind(this)} />
          }
        </Modal.Body>
      </Modal>
    );

  }
}

export default MissionModal;
