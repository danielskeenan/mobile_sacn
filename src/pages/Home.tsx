import {Title} from "@solidjs/meta";
import {Button, Card, Col, Container, Figure, Row} from "solid-bootstrap";
import Jumbotron from "@/components/Jumbotron";
import logo from "@/assets/mobile_sacn.svg";
import "./Home.scss";
import chanCheckImg from "@/assets/chancheck_dark.png";
import controlImg from "@/assets/control_keypad_dark.png";
import viewLevelsImg from "@/assets/viewlevels_grid_dark.png";
import {type Component} from "solid-js";
import {A} from "@solidjs/router";
import LINKS from "@/links.ts";

const Home: Component = () => {
    return (
        <main>
            <Title>Mobile sACN</Title>
            <Container>
                <Row>
                    <Jumbotron bg="body-secondary">
                        <h1 class="display-5">
                            <img src={logo} alt="" class="msacn-logo"/>&nbsp;Mobile sACN
                        </h1>
                        <p class="h2">Remote sACN Testing Tool</p>

                        <p>
                            Mobile sACN allows remote troubleshooting of sACN signals. Your mobile device connects to a
                            program on your computer that handles sACN traffic.
                        </p>

                        <ul class="list-inline">
                            <li class="list-inline-item">
                                <Button variant="success" as={A} href={LINKS.download}>Download</Button>
                            </li>

                            <li class="list-inline-item">
                                <Button variant="primary" as="a" href={LINKS.doc} target="_blank">
                                    User Manual
                                </Button>
                            </li>
                        </ul>
                    </Jumbotron>
                </Row>

                <Row class="screenshots">
                    <Col>
                        <Card>
                            <Card.Header>Channel Check</Card.Header>
                            <Card.Img variant="top" src={chanCheckImg} alt="Screenshot of channel check mode"/>
                            <Card.Body>
                                Run through addresses in any universe. Use per-address-priority to avoid affecting
                                ranges you're not interested in.
                            </Card.Body>
                        </Card>
                    </Col>
                    <Col>
                        <Card>
                            <Card.Header>Send Levels</Card.Header>
                            <Card.Img variant="top" src={controlImg}
                                      alt="Screenshot of live control mode's keypad interface"/>
                            <Card.Body>
                                Adjust multiple channels in one universe using faders or a simple command-line
                                interface.
                            </Card.Body>
                        </Card>
                    </Col>
                    <Col>
                        <Card>
                            <Card.Header>View Levels</Card.Header>
                            <Card.Img variant="top" src={viewLevelsImg} alt="Screenshot of view levels mode"/>
                            <Card.Body>
                                View active levels from multiple sources.
                            </Card.Body>
                        </Card>
                    </Col>
                </Row>
            </Container>
        </main>
    );
}

export default Home;