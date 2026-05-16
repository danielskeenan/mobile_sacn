import {createUniqueId, type ParentComponent, Suspense} from 'solid-js';
import {MetaProvider, Title} from "@solidjs/meta";
import {Container, Nav, Navbar} from "solid-bootstrap";
import logo from "./assets/mobile_sacn.svg";
import {BsGithub} from "solid-icons/bs";
import "./App.scss";
import LINKS from "@/links.ts";
import {A} from "@solidjs/router";
import {createEventListener} from "@solid-primitives/event-listener";

const App: ParentComponent = (props) => {
    // Color mode updates
    const prefersDarkMode = window.matchMedia("(prefers-color-scheme: dark)");
    const updateBsTheme = () => {
        document.documentElement.dataset.bsTheme = prefersDarkMode.matches ? "dark" : "light";
    };
    createEventListener(prefersDarkMode, "change", updateBsTheme);
    updateBsTheme();

    const navbarTopId = createUniqueId();

    return (
        <MetaProvider>
            <Title>Mobile sACN</Title>
            <header>
                <Navbar class="msacn-navbar-top" expand="lg" collapseOnSelect>
                    <Container>
                        <Navbar.Brand href={LINKS.home}>
                            <img class="msacn-logo" src={logo} alt=""/>&nbsp;Mobile sACN
                        </Navbar.Brand>
                        <Navbar.Toggle aria-controls={navbarTopId}/>
                        <Navbar.Collapse id={navbarTopId}>
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
                <Navbar class="msacn-navbar-bottom">
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
