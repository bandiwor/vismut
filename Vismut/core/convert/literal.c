#include "literal.h"
#include "../utils/overflow.h"
#include <assert.h>

VismutErrorType narrow_literal(const VismutSimpleValue from_value, const VismutTypeKind from_type,
                               const VismutTypeKind to_type, VismutSimpleValue *out_value) {
    if (from_type == VISMUT_TYPE_KIND_INT) {
        if (IsNumberOverflowed(from_value, to_type)) {
            return VISMUT_ERR_NUM_OVERFLOW;
        }
        switch (to_type) {
        case VISMUT_TYPE_KIND_I1:
            out_value->i = from_value.u;
            break;
        case VISMUT_TYPE_KIND_I8:
            out_value->i = from_value.u;
            break;
        case VISMUT_TYPE_KIND_I16:
            out_value->i = from_value.u;
            break;
        case VISMUT_TYPE_KIND_I32:
            out_value->i = from_value.u;
            break;
        case VISMUT_TYPE_KIND_I64:
            out_value->i = from_value.u;
            break;
        case VISMUT_TYPE_KIND_U8:
            out_value->u = from_value.u;
            break;
        case VISMUT_TYPE_KIND_U16:
            out_value->u = from_value.u;
            break;
        case VISMUT_TYPE_KIND_U32:
            out_value->u = from_value.u;
            break;
        case VISMUT_TYPE_KIND_U64:
            out_value->u = from_value.u;
            break;
        default:
            assert("Unreachable!");
            return VISMUT_ERR_UNREACHABLE;
        }
    } else if (from_type == VISMUT_TYPE_KIND_FLOAT) {
        switch (to_type) {
        case VISMUT_TYPE_KIND_F32:
        case VISMUT_TYPE_KIND_F64:
            out_value->f = from_value.f;
            return VISMUT_OK;
        default:
            assert("Unreachable!");
            return VISMUT_ERR_UNREACHABLE;
        }
    } else {
        assert("Unreachable!");
        return VISMUT_ERR_UNREACHABLE;
    }

    return VISMUT_OK;
}
