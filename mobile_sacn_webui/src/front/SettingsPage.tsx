import React, {useCallback, useContext} from "react";
import AppContext from "../common/Context";
import {Form} from "react-bootstrap";
import {LevelDisplayMode, levelDisplayString} from "../common/components/LevelDisplay";
import {SettingsTitle} from "./FrontTitle";

export default function SettingsPage() {
    const {levelDisplayMode, setLevelDisplayMode} = useContext(AppContext);
    const onLevelDisplayModeChange = useCallback((e: React.ChangeEvent<HTMLSelectElement>) => {
        setLevelDisplayMode(e.target.value as LevelDisplayMode);
    }, [setLevelDisplayMode]);

    return (
        <>
            <h1><SettingsTitle/></h1>
            <Form>
                <Form.Group className="mb-3">
                    <Form.Label>Level display mode</Form.Label>
                    <Form.Select value={levelDisplayMode} onChange={onLevelDisplayModeChange}>
                        <option value={LevelDisplayMode.PERCENT}>
                            Percent ("{levelDisplayString(127, LevelDisplayMode.PERCENT)}")
                        </option>
                        <option value={LevelDisplayMode.DECIMAL}>
                            Decimal ("{levelDisplayString(127, LevelDisplayMode.DECIMAL)}")
                        </option>
                        <option value={LevelDisplayMode.HEX}>
                            Hex ("{levelDisplayString(127, LevelDisplayMode.HEX)}")
                        </option>
                    </Form.Select>
                </Form.Group>
            </Form>
        </>
    );
}
