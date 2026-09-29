# D9 position measured by the drone, forwarded by the server. Topic: /d9/pose
# Drone frame and units (unknown yet); convert only in d9_core/coords.hpp.
# No heading on purpose: the drone does not supply one.
builtin_interfaces/Time stamp
float32 x
float32 y
