#define SHPP_FUNC_CHECK_NUM_PARAMS(params, num, func)                   \
  if (params.size() != num) {                                           \
    throw RunTimeError(                                                 \
        RunTimeError::ErrorCode::FUNC_PARAMS,                           \
        boost::format("%1% takes exactly %2% argument") % #func % num); \
  }

#define SHPP_FUNC_CHECK_NUM_PARAMS_UNTIL(params, num, func)             \
  if (params.size() > num) {                                            \
    throw RunTimeError(                                                 \
        RunTimeError::ErrorCode::FUNC_PARAMS,                           \
        boost::format("%1% takes at most %2% argument") % #func % num); \
  }

#define SHPP_FUNC_CHECK_NUM_PARAMS_AT_LEAST(params, num, func)           \
  if (params.size() < num) {                                             \
    throw RunTimeError(                                                  \
        RunTimeError::ErrorCode::FUNC_PARAMS,                            \
        boost::format("%1% takes at least %2% argument") % #func % num); \
  }

#define SHPP_FUNC_CHECK_NO_PARAMS(params, func)                         \
  if (params.size() != 0) {                                             \
    throw RunTimeError(RunTimeError::ErrorCode::FUNC_PARAMS,            \
                       boost::format("%1% takes no argument") % #func); \
  }

#define SHPP_FUNC_CHECK_PARAM_TYPE(param, param_name, obj_type)                \
  if (param->type() != Object::ObjectType::obj_type) {                         \
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,             \
                       boost::format("%1% must be " #obj_type) % #param_name); \
  }
