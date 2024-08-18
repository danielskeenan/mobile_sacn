import "./Jumbotron.scss";
import {ReactNode} from "react";

export default function Jumbotron(props: { children: ReactNode }) {
    return (
        <div className="jumbotron-outer">
            <div className="jumbotron-inner">
                {props.children}
            </div>
        </div>
    );
}
