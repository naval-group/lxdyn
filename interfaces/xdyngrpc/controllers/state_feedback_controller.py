"""Simple state feedback controller."""

import logging
from itertools import compress
from typing import Dict, List

import numpy as np
import yaml

from .controller import AbstractControllerModel, StatesEuler, clip, saturate_command, serve

SERVICE_NAME = "state-feedback-controller"

logging.basicConfig(
    format="%(asctime)s,%(msecs)d ["
    + SERVICE_NAME
    + "] - %(levelname)-4s [%(filename)s:%(lineno)d] %(message)s",
    datefmt="%d-%m-%Y:%H:%M:%S",
)
LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


class StateFeedbackController(AbstractControllerModel):
    """Computes the command according to given states list and gains.
    Cf. https://en.wikipedia.org/wiki/Full_state_feedback
    """

    def __init__(self, parameters: str, t0: float):
        """Initialize parameters from gRPC's set_parameters."""
        param = yaml.safe_load(parameters)
        self.K = np.array(param["K matrix"])
        self.input_states = param["input states"]
        self.setpoints_bind = param.get("setpoints bind", {})
        self.command_names = param["commands"]
        self.previous_command = param.get("initial position", [0.0] * len(self.command_names))
        self.pos_saturation = param.get(
            "output position saturation", [None] * len(self.command_names)
        )
        self.spd_saturation = param.get("output speed saturation", [None] * len(self.command_names))
        self.integral_states = param.get("integral states with saturation", dict())

        assert len(self.previous_command) == len(self.command_names), (
            "The number of initial position is not equal to"
            f"the number of commands: there are {len(self.previous_command)} position but there "
            f"are {len(self.command_names)} commands."
        )

        assert len(self.pos_saturation) == len(self.command_names), (
            "The number of output position saturation is not equal to"
            f"the number of commands: there are {len(self.pos_saturation)} position saturation "
            f"but there are {len(self.command_names)} commands. Fill with 'null' eventually."
        )

        assert len(self.spd_saturation) == len(self.command_names), (
            "The number of output speed saturation is not equal to"
            f"the number of commands: there are {len(self.spd_saturation)} speed saturation "
            f"but there are {len(self.command_names)} commands. Fill with 'null' eventually."
        )

        integral_states_not_in_input_states = [
            state not in self.input_states for state in self.integral_states
        ]
        if any(integral_states_not_in_input_states):
            error_int_states = list(
                compress(self.integral_states, integral_states_not_in_input_states)
            )
            raise AssertionError(
                "The following states need to be integrated by the controller "
                "(they are in 'integral states with saturation') but they "
                "couldn't be found in 'input states': either remove them from "
                "'integral states with saturation' or add them to 'input states':"
                f" {error_int_states}"
            )

        self.integrals = {k: 0.0 for k in self.integral_states}

        assert self.K.shape[0] == len(self.command_names), (
            "The number of rows in gain matrix K is not equal to "
            f"the number of commands: K has {self.K.shape[0]} rows but there "
            f"are {len(self.command_names)} commands."
        )

        assert self.K.shape[1] == len(self.input_states) + len(self.integral_states), (
            "The number of columns in gain matrix K is not equal to "
            f"the number of input (and integral) states: K has {self.K.shape[1]} columns "
            f"but there are {len(self.input_states) + len(self.integral_states)} states."
        )

        super().__init__(t0, param["dt"])

    def get_angle_representation(self) -> str:
        """Which method should be called to get the commands computed by the controller?

        If this method returns 'QUATERNION', then the solver will call get_commands_quaternion.
        If this method returns 'EULER_321', then solver will call get_commands_euler_321.

        """
        return "EULER_321"

    def get_command_names(self) -> List[str]:
        """Return the name(s) of the controller outputs (commands).

        This will be used by the solver (e.g., xdyn) to set the value of the corresponding
        variables.
        The names are assumed to be in the same order as the numerical values computed by
        'get_commands*'
        and in the same order as the lines in matrix K.

        Returns
        -------
        - commands (List[str]): commands computed by this controller
        """
        LOGGER.info(self.command_names)
        return self.command_names

    def get_setpoint_names(self) -> List[str]:
        """Return the name(s) of the controller inputs (setpoints).

        These setpoints are given by the simulatorcd fvd

        Returns
        -------
        - setpoints (Dict[str,float]): the strings must correspond to the commands used by
        xdyn's controlled forces
        """
        return self.setpoints_bind.values()

    def get_plant_output(self, states: StatesEuler) -> np.ndarray:
        """Calculates the list of states used to compute the command."""
        return np.array([states.__getattribute__(s) for s in self.input_states])

    def get_state_setpoints(self, setpoints: Dict[str, float]) -> np.ndarray:
        """Get the setpoint for each state. If input argument 'setpoints' does not contain
        a given state, its setpoint will default to zero.

        Parameters
        ----------
        - setpoints (Dict[str,float]): controller inputs (setpoints) given by the simulator
        """
        return np.array(
            [setpoints.get(self.setpoints_bind.get(s, None), 0.0) for s in self.input_states]
        )

    def get_commands_euler_321(
        self,
        states: StatesEuler,
        dstates_dt: StatesEuler,
        setpoints: List[float],
    ) -> Dict[str, float]:
        """Calculate the commands using the state feedback formula u = -K * (x - x_desired).
        If some integral states are specified, x is augmented with the computed integral values

            Parameters
            ----------
            - states (StatesQuaternion): latest ship states
            - dstates_dt (StatesQuaternion): ship states derivative at the
                                             previous timestep
            - setpoints (Dict[str,float]): controller inputs (setpoints)

            Returns
            -------
            - commands (Dict[str,float]): commands used by xdyn's controlled
                                          forces
        """
        setpoints_dict = dict(zip(self.get_setpoint_names(), setpoints))
        error = self.get_plant_output(states) - self.get_state_setpoints(setpoints_dict)

        # Integral values computation
        for state in self.integral_states:
            state_err = error[self.input_states.index(state)]
            self.integrals[state] += state_err * self.dt
            if self.integral_states[state] is not None:
                self.integrals[state] = clip(
                    self.integrals[state], -self.integral_states[state], self.integral_states[state]
                )

        commands = -self.K @ np.append(error, np.fromiter(self.integrals.values(), dtype=float))

        commands = [
            saturate_command(
                x,
                self.previous_command[ind],
                self.dt,
                self.pos_saturation[ind],
                self.spd_saturation[ind],
            )
            for ind, x in enumerate(commands)
        ]

        self.previous_command = commands

        return dict(zip(self.command_names, commands))


# Start the gRPC server loop
if __name__ == "__main__":
    LOGGER.info("Starting gRPC state feedback controller")
    serve(StateFeedbackController)
