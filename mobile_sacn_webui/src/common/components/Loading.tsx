import {Component} from "solid-js";
import {Alert, Spinner} from "solid-bootstrap";

const Loading: Component = ()=>{
    return (
      <Alert variant="light">
          <Spinner animation="border" size="sm"/>&nbsp;Loading...
      </Alert>
    );
}

export default Loading;
