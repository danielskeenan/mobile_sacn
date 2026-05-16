import {type Component, createMemo, type ParentProps} from "solid-js";
import "./Jumbotron.scss";

interface JumbotronProps extends ParentProps {
    bg?: string;
}

const Jumbotron: Component<JumbotronProps> = (props) => {
    const classes = createMemo(() => {
        const classList = ["jumbotron"]
        if (props.bg) {
            classList.push(`bg-${props.bg}`);
        }
        return classList.join(' ');
    });

    return (
        <div class={classes()}>
            {props.children}
        </div>
    )
}

export default Jumbotron;