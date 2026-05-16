import {type ParentComponent, Suspense} from 'solid-js';
import {MetaProvider, Title} from "@solidjs/meta";
import {Container, Nav, Navbar} from "solid-bootstrap";
import logo from "./assets/mobile_sacn.svg";
import {BsGithub} from "solid-icons/bs";
import "./App.scss";
import LINKS from "@/links.ts";
import {A} from "@solidjs/router";

const App: ParentComponent = (props) => {
    return (
        <MetaProvider>
            <Title>Mobile sACN</Title>
            <header>
                <Navbar class="msacn-navbar-top" expand="lg">
                    <Container>
                        <Navbar.Brand href={LINKS.home}>
                            <img class="msacn-logo" src={logo} alt=""/>&nbsp;Mobile sACN
                        </Navbar.Brand>
                        <Navbar.Toggle/>
                        <Navbar.Collapse>
                            <Nav>
                                <Nav.Link as={A} href={LINKS.download}>Download</Nav.Link>
                                <Nav.Link href={"/doc/index.html"} target="_blank">Manual</Nav.Link>
                            </Nav>
                        </Navbar.Collapse>
                    </Container>
                </Navbar>
            </header>

            <Suspense>{props.children}</Suspense>

            <footer>
                <Navbar bg="light" class="msacn-navbar-bottom">
                    <Container>
                        <div class="msacn-github-link">
                            <a href="https://github.com/danielskeenan/mobile_sacn" target="_blank">
                                <BsGithub/>&nbsp;Development on GitHub
                            </a>
                        </div>
                    </Container>
                </Navbar>
            </footer>
        </MetaProvider>
    );
};

export default App;
