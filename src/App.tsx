import {type ParentComponent, Suspense} from 'solid-js';
import {MetaProvider, Title} from "@solidjs/meta";
import {Container, Navbar} from "solid-bootstrap";
import logo from "./assets/mobile_sacn.svg";
import {BsGithub} from "solid-icons/bs";
import "./App.scss";
import LINKS from "@/links.ts";

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
                    </Container>
                </Navbar>
            </header>

            <Suspense>{props.children}</Suspense>

            <footer>
                <Navbar bg="light" class="msacn-navbar-bottom">
                    <Container>
                        <div class="msacn-github-link">
                            <a href="https://github.com/danielskeenan/mobile_sacn">
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
