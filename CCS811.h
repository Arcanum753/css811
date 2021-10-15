#include "CCS811_defs.h"

int8_t ccs811_init(struct ccs811_dev *dev);
void css811_setEnvironmentalData(struct ccs811_dev *dev);
int8_t ccs811_get_data(struct ccs811_dev *dev, struct ccs811_data *data);

bool ccs811_check_error(struct ccs811_dev *dev);
bool isDataReady(struct ccs811_dev *dev);
