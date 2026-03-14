import {Component} from "solid-js";
import {ListGroup} from "solid-bootstrap";
import {A} from "@solidjs/router";
import ChanCheckTitle from "@/pages/transmit/ChanCheckTitle";
import LINKS from "@/links";
import TransmitLevelsTitle from "@/pages/transmit/TransmitLevelsTitle";
import ReceiveLevelsTitle from "@/pages/receive/ReceiveLevelsTitle";
import setPageTitle from "@/common/setPageTitle";

const Home: Component = () => {
    setPageTitle();

    return (
        <>
            <ListGroup>
                <ListGroup.Item as={A} href={LINKS.transmit_levels} action>
                    <TransmitLevelsTitle/>
                </ListGroup.Item>
                <ListGroup.Item as={A} href={LINKS.transmit_chancheck} action>
                    <ChanCheckTitle/>
                </ListGroup.Item>
                <ListGroup.Item as={A} href={LINKS.receive_levels} action>
                    <ReceiveLevelsTitle/>
                </ListGroup.Item>
            </ListGroup>
        </>
    );
};

export default Home;
