import {Container, Row} from "solid-bootstrap";
import {A} from "@solidjs/router";
import LINKS from "@/links.ts";

export default function NotFound() {
    return (
        <main>
            <Container>
                <Row>
                    <h1>File not found.</h1>
                    <p>
                        <A href={LINKS.home}>Go to the homepage.</A>
                    </p>
                </Row>
            </Container>
        </main>
    );
}
