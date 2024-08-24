import {useContext} from "react";
import AppContext from "../common/Context.ts";
import {SettingsTitle} from "./FrontTitle.tsx";
import {Form} from "react-bootstrap";
import {LevelDisplayMode, levelDisplayString} from "../common/levelDisplay.ts";

export function Component() {
    const context = useContext(AppContext);

    return (
        <>
            <h1><SettingsTitle/></h1>
            <Form>
                <Form.Group className="mb-3">
                    <Form.Label className="me-3">Dark Mode</Form.Label>
                    <Form.Check
                        inline
                        type="switch"
                        checked={context.darkMode}
                        onChange={() => context.setDarkMode(!context.darkMode)}
                    />
                </Form.Group>
                <Form.Group className="mb-3">
                    <Form.Label>Level display mode</Form.Label>
                    <div className="ms-5">
                        <Form.Check
                            type="radio"
                            label={`Percent ("${levelDisplayString(127, LevelDisplayMode.PERCENT)}")`}
                            checked={context.levelDisplayMode == LevelDisplayMode.PERCENT}
                            onChange={() => context.setLevelDisplayMode(LevelDisplayMode.PERCENT)}
                        />
                        <Form.Check
                            type="radio"
                            label={`Decimal ("${levelDisplayString(127, LevelDisplayMode.DECIMAL)}")`}
                            checked={context.levelDisplayMode == LevelDisplayMode.DECIMAL}
                            onChange={() => context.setLevelDisplayMode(LevelDisplayMode.DECIMAL)}
                        />
                        <Form.Check
                            type="radio"
                            label={`Hex ("${levelDisplayString(127, LevelDisplayMode.HEX)}")`}
                            checked={context.levelDisplayMode == LevelDisplayMode.HEX}
                            onChange={() => context.setLevelDisplayMode(LevelDisplayMode.HEX)}
                        />
                    </div>
                </Form.Group>
            </Form>
        </>
    );
}
