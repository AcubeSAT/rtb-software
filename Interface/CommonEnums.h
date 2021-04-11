#ifndef RADIATIONINTERFACE_COMMONENUMS_H
#define RADIATIONINTERFACE_COMMONENUMS_H

#ifdef __cplusplus
namespace parameters {
#endif

enum CANSpeed {
    baud125kbps = 0,
    baud250kbps = 1,
    baud500kbps,
    baud750kbps,
    baud1000kbps,
};

#ifdef __cplusplus
}
#endif

#endif //RADIATIONINTERFACE_COMMONENUMS_H
