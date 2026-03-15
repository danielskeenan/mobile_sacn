import setPageTitle from "@/common/setPageTitle";
import LINKS from "@/links";
import ReceiveLevelsTitle from "@/pages/receive/levels/ReceiveLevelsTitle";
import ChanCheckTitle from "@/pages/transmit/ChanCheckTitle";
import TransmitLevelsTitle from "@/pages/transmit/TransmitLevelsTitle";
import {A} from "@solidjs/router";
import {ListGroup} from "solid-bootstrap";
import {Component} from "solid-js";

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
