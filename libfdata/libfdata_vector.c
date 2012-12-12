/*
 * The vector functions
 *
 * Copyright (c) 2010-2012, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include "libfdata_definitions.h"
#include "libfdata_libcdata.h"
#include "libfdata_libcerror.h"
#include "libfdata_libcnotify.h"
#include "libfdata_libfcache.h"
#include "libfdata_range.h"
#include "libfdata_types.h"
#include "libfdata_vector.h"

#define libfdata_vector_calculate_cache_entry_index( element_index, number_of_cache_entries ) \
	element_index % number_of_cache_entries

/* Initializes the vector
 *
 * If the flag LIBFDATA_FLAG_DATA_HANDLE_MANAGED is set the vector
 * takes over management of the data handle and the data handle is freed when
 * no longer needed
 *
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_initialize(
     libfdata_vector_t **vector,
     size64_t element_size,
     intptr_t *data_handle,
     int (*free_data_handle)(
            intptr_t **data_handle,
            libcerror_error_t **error ),
     int (*clone_data_handle)(
            intptr_t **destination_data_handle,
            intptr_t *source_data_handle,
            libcerror_error_t **error ),
     int (*read_element_data)(
            intptr_t *data_handle,
            intptr_t *file_io_handle,
            libfdata_vector_t *vector,
            libfcache_cache_t *cache,
            int element_index,
            int element_data_file_index,
            off64_t element_data_offset,
            size64_t element_data_size,
            uint32_t element_data_flags,
            uint8_t read_flags,
            libcerror_error_t **error ),
     int (*write_element_data)(
            intptr_t *data_handle,
            intptr_t *file_io_handle,
            libfdata_vector_t *vector,
            libfcache_cache_t *cache,
            int element_index,
            int element_data_file_index,
            off64_t element_data_offset,
            size64_t element_data_size,
            uint32_t element_data_flags,
            uint8_t write_flags,
            libcerror_error_t **error ),
     uint8_t flags,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_initialize";

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	if( *vector != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid vector value already set.",
		 function );

		return( -1 );
	}
	if( element_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid element size value zero or less.",
		 function );

		return( -1 );
	}
	internal_vector = memory_allocate_structure(
	                   libfdata_internal_vector_t );

	if( internal_vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create vector.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     internal_vector,
	     0,
	     sizeof( libfdata_internal_vector_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear vector.",
		 function );

		goto on_error;
	}
	if( libcdata_array_initialize(
	     &( internal_vector->segments_array ),
	     0,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create segments array.",
		 function );

		goto on_error;
	}
	internal_vector->element_size       = element_size;
	internal_vector->timestamp          = libfcache_date_time_get_timestamp();
	internal_vector->flags             |= flags;
	internal_vector->data_handle        = data_handle;
	internal_vector->free_data_handle   = free_data_handle;
	internal_vector->clone_data_handle  = clone_data_handle;
	internal_vector->read_element_data  = read_element_data;
	internal_vector->write_element_data = write_element_data;

	*vector = (libfdata_vector_t *) internal_vector;

	return( 1 );

on_error:
	if( internal_vector != NULL )
	{
		memory_free(
		 internal_vector );
	}
	return( -1 );
}

/* Frees the vector
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_free(
     libfdata_vector_t **vector,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_free";
	int result                                  = 1;

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	if( *vector != NULL )
	{
		internal_vector = (libfdata_internal_vector_t *) *vector;
		*vector         = NULL;

		if( libcdata_array_free(
		     &( internal_vector->segments_array ),
		     (int (*)(intptr_t **, libcerror_error_t **)) &libfdata_range_free,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free the segments array.",
			 function );

			result = -1;
		}
		if( ( internal_vector->flags & LIBFDATA_FLAG_DATA_HANDLE_MANAGED ) != 0 )
		{
			if( internal_vector->data_handle != NULL )
			{
				if( internal_vector->free_data_handle == NULL )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
					 "%s: invalid vector - missing free data handle function.",
					 function );

					result = -1;
				}
				else if( internal_vector->free_data_handle(
				          &( internal_vector->data_handle ),
				          error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
					 "%s: unable to free data handle.",
					 function );

					result = -1;
				}
			}
		}
		memory_free(
		 internal_vector );
	}
	return( result );
}

/* Clones (duplicates) the vector
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_clone(
     libfdata_vector_t **destination_vector,
     libfdata_vector_t *source_vector,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_source_vector = NULL;
	intptr_t *destination_data_handle                  = NULL;
	static char *function                              = "libfdata_vector_clone";

	if( destination_vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid destination vector.",
		 function );

		return( -1 );
	}
	if( *destination_vector != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid destination vector value already set.",
		 function );

		return( -1 );
	}
	if( source_vector == NULL )
	{
		*destination_vector = source_vector;

		return( 1 );
	}
	internal_source_vector = (libfdata_internal_vector_t *) source_vector;

	if( internal_source_vector->data_handle != NULL )
	{
		if( internal_source_vector->free_data_handle == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			 "%s: invalid source vector - missing free data handle function.",
			 function );

			goto on_error;
		}
		if( internal_source_vector->clone_data_handle == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			 "%s: invalid source vector - missing clone data handle function.",
			 function );

			goto on_error;
		}
		if( internal_source_vector->clone_data_handle(
		     &destination_data_handle,
		     internal_source_vector->data_handle,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to clone data handle.",
			 function );

			goto on_error;
		}
	}
	if( libfdata_vector_initialize(
	     destination_vector,
	     internal_source_vector->element_size,
	     destination_data_handle,
	     internal_source_vector->free_data_handle,
	     internal_source_vector->clone_data_handle,
	     internal_source_vector->read_element_data,
	     internal_source_vector->write_element_data,
	     LIBFDATA_FLAG_DATA_HANDLE_MANAGED,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create destination vector.",
		 function );

		goto on_error;
	}
/* TODO clone data ranges */
	return( 1 );

on_error:
	if( destination_data_handle != NULL )
	{
		internal_source_vector->free_data_handle(
		 &destination_data_handle,
		 NULL );
	}
	return( -1 );
}

/* Retrieves the element index for a specific offset
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_get_element_index_at_offset(
     libfdata_vector_t *vector,
     off64_t value_offset,
     int *element_index,
     size_t *element_offset,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	libfdata_range_t *segment_data_range        = NULL;
	static char *function                       = "libfdata_vector_get_element_index_at_offset";
	off64_t segment_data_offset                 = 0;
	uint64_t calculated_element_index           = 0;
	int number_of_segments                      = 0;
	int segment_index                           = 0;

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( internal_vector->element_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid vector - element size value out of bounds.",
		 function );

		return( -1 );
	}
	if( value_offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid value offset value less than zero.",
		 function );

		return( -1 );
	}
	if( element_index == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid element index.",
		 function );

		return( -1 );
	}
	if( element_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid element offset.",
		 function );

		return( -1 );
	}
	if( libcdata_array_get_number_of_entries(
	     internal_vector->segments_array,
	     &number_of_segments,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of segments.",
		 function );

		return( -1 );
	}
	if( number_of_segments <= 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid number of segments value out of bounds.",
		 function );

		return( -1 );
	}
	for( segment_index = 0;
	     segment_index < number_of_segments;
	     segment_index++ )
	{
		if( libcdata_array_get_entry_by_index(
		     internal_vector->segments_array,
		     segment_index,
		     (intptr_t **) &segment_data_range,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve segment data range: %d from array.",
			 function,
			 segment_index );

			return( -1 );
		}
		if( segment_data_range == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			 "%s: missing segment data range: %d.",
			 function,
			 segment_index );

			return( -1 );
		}
/* TODO what about compressed data ranges */
		if( value_offset < (off64_t)( segment_data_offset + segment_data_range->size ) )
		{
			break;
		}
		segment_data_offset += (off64_t) segment_data_range->size;
	}
	if( segment_index >= number_of_segments )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid segment index value out of bounds.",
		 function );

		return( -1 );
	}
	calculated_element_index  = segment_data_offset + value_offset;
	calculated_element_index /= internal_vector->element_size;

	if( calculated_element_index > (uint64_t) INT_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid element index value exceeds maximum.",
		 function );

		return( -1 );
	}
	value_offset -= (off64_t) ( calculated_element_index * internal_vector->element_size );

	if( ( value_offset < 0 )
	 || ( value_offset > (off64_t) SSIZE_MAX ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid element offset value out of bounds.",
		 function );

		return( -1 );
	}
	*element_index  = (int) calculated_element_index;
	*element_offset = (size_t) value_offset;

	return( 1 );
}

/* Segment functions
 */

/* Empties the segments
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_empty_segments(
     libfdata_vector_t *vector,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_empty_segments";

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( libcdata_array_empty(
	     internal_vector->segments_array,
	     (int (*)(intptr_t **, libcerror_error_t **)) &libfdata_range_free,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to empty segments array.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Resizes the segments
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_resize_segments(
     libfdata_vector_t *vector,
     int number_of_segments,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_resize_segments";

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( libcdata_array_resize(
	     internal_vector->segments_array,
	     number_of_segments,
	     (int (*)(intptr_t **, libcerror_error_t **)) &libfdata_range_free,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_RESIZE_FAILED,
		 "%s: unable to resize segments array.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the number of segments of the vector
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_get_number_of_segments(
     libfdata_vector_t *vector,
     int *number_of_segments,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_get_number_of_segments";

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( libcdata_array_get_number_of_entries(
	     internal_vector->segments_array,
	     number_of_segments,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of entries from segments array.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the offset and size of a specific segment
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_get_segment_by_index(
     libfdata_vector_t *vector,
     int segment_index,
     int *segment_file_index,
     off64_t *segment_offset,
     size64_t *segment_size,
     uint32_t *segment_flags,
     libcerror_error_t **error )
{
	libfdata_range_t *segment_data_range        = NULL;
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_get_segment_by_index";

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( libcdata_array_get_entry_by_index(
	     internal_vector->segments_array,
	     segment_index,
	     (intptr_t **) &segment_data_range,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve entry: %d from segments array.",
		 function,
		 segment_index );

		return( -1 );
	}
	if( libfdata_range_get(
	     segment_data_range,
	     segment_file_index,
	     segment_offset,
	     segment_size,
	     segment_flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve segment: %d data range values.",
		 function,
		 segment_index );

		return( -1 );
	}
	return( 1 );
}

/* Sets the offset and size of a specific segment
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_set_segment_by_index(
     libfdata_vector_t *vector,
     int segment_index,
     int segment_file_index,
     off64_t segment_offset,
     size64_t segment_size,
     uint32_t segment_flags,
     libcerror_error_t **error )
{
	libfdata_range_t *segment_data_range        = NULL;
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_set_segment_by_index";
	off64_t previous_segment_offset             = 0;
	size64_t previous_segment_size              = 0;
	uint32_t previous_segment_flags             = 0;
	int previous_segment_file_index             = 0;

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( libcdata_array_get_entry_by_index(
	     internal_vector->segments_array,
	     segment_index,
	     (intptr_t **) &segment_data_range,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve entry: %d from segments array.",
		 function,
		 segment_index );

		return( -1 );
	}
	if( segment_data_range == NULL )
	{
		if( libfdata_range_initialize(
		     &segment_data_range,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create segment data range.",
			 function );

			return( -1 );
		}
		if( libcdata_array_set_entry_by_index(
		     internal_vector->segments_array,
		     segment_index,
		     (intptr_t *) segment_data_range,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set entry: %d to segments array.",
			 function,
			 segment_index );

			libfdata_range_free(
			 &segment_data_range,
			 NULL );

			return( -1 );
		}
	}
	else
	{
		if( libfdata_range_get(
		     segment_data_range,
		     &previous_segment_file_index,
		     &previous_segment_offset,
		     &previous_segment_size,
		     &previous_segment_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve segment: %d data range values.",
			 function,
			 segment_index );

			return( -1 );
		}
		internal_vector->data_size -= previous_segment_size;
	}
	if( libfdata_range_set(
	     segment_data_range,
	     segment_file_index,
	     segment_offset,
	     segment_size,
	     segment_flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set segment data range values.",
		 function );

		return( -1 );
	}
	internal_vector->data_size += segment_size;

	return( 1 );
}

/* Appends a segment
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_append_segment(
     libfdata_vector_t *vector,
     int *segment_index,
     int segment_file_index,
     off64_t segment_offset,
     size64_t segment_size,
     uint32_t segment_flags,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	libfdata_range_t *segment_data_range        = NULL;
	static char *function                       = "libfdata_vector_append_segment";

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( libfdata_range_initialize(
	     &segment_data_range,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create segment data range.",
		 function );

		goto on_error;
	}
	if( libfdata_range_set(
	     segment_data_range,
	     segment_file_index,
	     segment_offset,
	     segment_size,
	     segment_flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set segment data range values.",
		 function );

		goto on_error;
	}
	if( libcdata_array_append_entry(
	     internal_vector->segments_array,
	     segment_index,
	     (intptr_t *) segment_data_range,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append data range to segments array.",
		 function );

		goto on_error;
	}
	internal_vector->data_size += segment_size;

	return( 1 );

on_error:
	if( segment_data_range != NULL )
	{
		libfdata_range_free(
		 &segment_data_range,
		 NULL );
	}
	return( -1 );
}

/* Vector element functions
 */

/* Retrieves the number of elements of the vector
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_get_number_of_elements(
     libfdata_vector_t *vector,
     int *number_of_elements,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_get_number_of_elements";
	size64_t safe_number_of_elements            = 0;

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( internal_vector->element_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid vector - element size value out of bounds.",
		 function );

		return( -1 );
	}
	if( number_of_elements == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid number of elements.",
		 function );

		return( -1 );
	}
	safe_number_of_elements = internal_vector->data_size
	                        / internal_vector->element_size;

	if( safe_number_of_elements > (size64_t) INT_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: number of elements value out of bounds.",
		 function );

		return( -1 );
	}
	*number_of_elements = (int) safe_number_of_elements;

	return( 1 );
}

/* Retrieves the value of a specific element
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_get_element_value_by_index(
     libfdata_vector_t *vector,
     intptr_t *file_io_handle,
     libfcache_cache_t *cache,
     int element_index,
     intptr_t **element_value,
     uint8_t read_flags,
     libcerror_error_t **error )
{
	libfcache_cache_value_t *cache_value        = NULL;
	libfdata_internal_vector_t *internal_vector = NULL;
	libfdata_range_t *segment_data_range        = NULL;
	static char *function                       = "libfdata_vector_get_element_value_by_index";
	off64_t cache_value_offset                  = (off64_t) -1;
	off64_t element_data_offset                 = 0;
	time_t cache_value_timestamp                = 0;
	int cache_entry_index                       = -1;
	int cache_value_file_index                  = -1;
	int number_of_cache_entries                 = 0;
	int number_of_segments                      = 0;
	int result                                  = 0;
	int segment_index                           = 0;

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( internal_vector->read_element_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid vector - missing read element data function.",
		 function );

		return( -1 );
	}
	if( internal_vector->element_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid vector - element size value out of bounds.",
		 function );

		return( -1 );
	}
	if( ( internal_vector->data_size == 0 )
	 || ( internal_vector->data_size > (off64_t) INT64_MAX ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid vector - data size value out of bounds.",
		 function );

		return( -1 );
	}
	if( element_index < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid element index value out of bounds.",
		 function );

		return( -1 );
	}
	element_data_offset = (off64_t) ( element_index * internal_vector->element_size );

	if( element_data_offset > (off64_t) internal_vector->data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid element index value out of bounds.",
		 function );

		return( -1 );
	}
	if( libcdata_array_get_number_of_entries(
	     internal_vector->segments_array,
	     &number_of_segments,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of segments.",
		 function );

		return( -1 );
	}
	if( number_of_segments <= 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid number of segments value out of bounds.",
		 function );

		return( -1 );
	}
	for( segment_index = 0;
	     segment_index < number_of_segments;
	     segment_index++ )
	{
		if( libcdata_array_get_entry_by_index(
		     internal_vector->segments_array,
		     segment_index,
		     (intptr_t **) &segment_data_range,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve segment data range: %d from array.",
			 function,
			 segment_index );

			return( -1 );
		}
		if( segment_data_range == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			 "%s: missing segment data range: %d.",
			 function,
			 segment_index );

			return( -1 );
		}
/* TODO what about compressed data ranges */
		if( element_data_offset < (off64_t) segment_data_range->size )
		{
			element_data_offset += segment_data_range->offset;

			break;
		}
		element_data_offset -= segment_data_range->size;
	}
	if( segment_index >= number_of_segments )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid segment index value out of bounds.",
		 function );

		return( -1 );
	}
	if( libfcache_cache_get_number_of_entries(
	     cache,
	     &number_of_cache_entries,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of cache entries.",
		 function );

		return( -1 );
	}
	if( number_of_cache_entries <= 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid number of cache entries value out of bounds.",
		 function );

		return( -1 );
	}
	if( ( read_flags & LIBFDATA_READ_FLAG_IGNORE_CACHE ) == 0 )
	{
		cache_entry_index = libfdata_vector_calculate_cache_entry_index(
		                     element_index,
		                     number_of_cache_entries );

		if( libfcache_cache_get_value_by_index(
		     cache,
		     cache_entry_index,
		     &cache_value,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve cache entry: %d from cache.",
			 function,
			 cache_entry_index );

			return( -1 );
		}
		if( cache_value != NULL )
		{
			if( libfcache_cache_value_get_identifier(
			     cache_value,
			     &cache_value_file_index,
			     &cache_value_offset,
			     &cache_value_timestamp,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve cache value identifier.",
				 function );

				return( -1 );
			}
		}
		if( ( element_data_offset == cache_value_offset )
		 && ( internal_vector->timestamp == cache_value_timestamp ) )
		{
			result = 1;
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			if( result == 0 )
			{
				libcnotify_printf(
				 "%s: cache: 0x%08" PRIjx " miss (%d out of %d)\n",
				 function,
				 (intptr_t) cache,
				 cache_entry_index,
				 number_of_cache_entries );
			}
			else
			{
				libcnotify_printf(
				 "%s: cache: 0x%08" PRIjx " hit (%d out of %d)\n",
				 function,
				 (intptr_t) cache,
				 cache_entry_index,
				 number_of_cache_entries );
			}
		}
#endif
	}
	if( result == 0 )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: reading element data at offset: %" PRIi64 " (0x%08" PRIx64 ") of size: %" PRIu64 "\n",
			 function,
			 element_data_offset,
			 element_data_offset,
			 internal_vector->element_size );
		}
#endif
/* TODO refactor */
		if( internal_vector->read_element_data(
		     internal_vector->data_handle,
		     file_io_handle,
		     vector,
		     cache,
		     element_index,
		     0,
		     element_data_offset,
		     internal_vector->element_size,
		     0,
		     read_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read element data at offset: %" PRIi64 ".",
			 function,
			 element_data_offset );

			return( -1 );
		}
		cache_entry_index = libfdata_vector_calculate_cache_entry_index(
		                     element_index,
		                     number_of_cache_entries );

		if( libfcache_cache_get_value_by_index(
		     cache,
		     cache_entry_index,
		     &cache_value,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve cache entry: %d from cache.",
			 function,
			 cache_entry_index );

			return( -1 );
		}
		if( cache_value != NULL )
		{
			if( libfcache_cache_value_get_identifier(
			     cache_value,
			     &cache_value_file_index,
			     &cache_value_offset,
			     &cache_value_timestamp,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve cache value identifier.",
				 function );

				return( -1 );
			}
		}
		if( ( element_data_offset != cache_value_offset )
		 || ( internal_vector->timestamp != cache_value_timestamp ) )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			 "%s: missing cache value.",
			 function );

			return( -1 );
		}
	}
	if( libfcache_cache_value_get_value(
	     cache_value,
	     element_value,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve element value.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the value an element at a specific offset
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_get_element_value_at_offset(
     libfdata_vector_t *vector,
     intptr_t *file_io_handle,
     libfcache_cache_t *cache,
     off64_t value_offset,
     intptr_t **element_value,
     uint8_t read_flags,
     libcerror_error_t **error )
{
	static char *function = "libfdata_vector_get_element_value_at_offset";
	size_t element_offset = 0;
	int element_index     = 0;

	if( libfdata_vector_get_element_index_at_offset(
	     vector,
	     value_offset,
	     &element_index,
	     &element_offset,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve element index at offset: %" PRIi64 ".",
		 function,
		 value_offset );

		return( -1 );
	}
	if( libfdata_vector_get_element_value_by_index(
	     vector,
	     file_io_handle,
	     cache,
	     element_index,
	     element_value,
	     read_flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve element: %d value.",
		 function,
		 element_index );

		return( -1 );
	}
	return( 1 );
}

/* Sets the value of a specific element
 *
 * If the flag LIBFDATA_VECTOR_ELEMENT_VALUE_FLAG_MANAGED is set the vector
 * takes over management of the value and the value is freed when
 * no longer needed.
 *
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_set_element_value_by_index(
     libfdata_vector_t *vector,
     libfcache_cache_t *cache,
     int element_index,
     intptr_t *element_value,
     int (*free_element_value)(
            intptr_t **element_value,
            libcerror_error_t **error ),
     uint8_t flags,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	libfdata_range_t *segment_data_range        = NULL;
	static char *function                       = "libfdata_vector_set_element_value_by_index";
	off64_t element_data_offset                 = 0;
	int cache_entry_index                       = -1;
	int number_of_cache_entries                 = 0;
	int number_of_segments                      = 0;
	int segment_index                           = 0;

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( internal_vector->element_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid vector - element size value out of bounds.",
		 function );

		return( -1 );
	}
	if( ( internal_vector->data_size == 0 )
	 || ( internal_vector->data_size > (off64_t) INT64_MAX ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid vector - data size value out of bounds.",
		 function );

		return( -1 );
	}
	if( element_index < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid element index value out of bounds.",
		 function );

		return( -1 );
	}
	element_data_offset = (off64_t) ( element_index * internal_vector->element_size );

	if( element_data_offset > (off64_t) internal_vector->data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid element index value out of bounds.",
		 function );

		return( -1 );
	}
	if( libcdata_array_get_number_of_entries(
	     internal_vector->segments_array,
	     &number_of_segments,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of segments.",
		 function );

		return( -1 );
	}
	if( number_of_segments <= 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid number of segments value out of bounds.",
		 function );

		return( -1 );
	}
	for( segment_index = 0;
	     segment_index < number_of_segments;
	     segment_index++ )
	{
		if( libcdata_array_get_entry_by_index(
		     internal_vector->segments_array,
		     segment_index,
		     (intptr_t **) &segment_data_range,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve segment data range: %d from array.",
			 function,
			 segment_index );

			return( -1 );
		}
		if( segment_data_range == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			 "%s: missing segment data range: %d.",
			 function,
			 segment_index );

			return( -1 );
		}
/* TODO what about compressed data ranges */
		if( element_data_offset < (off64_t) segment_data_range->size )
		{
			element_data_offset += segment_data_range->offset;

			break;
		}
		element_data_offset -= segment_data_range->size;
	}
	if( segment_index >= number_of_segments )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid segment index value out of bounds.",
		 function );

		return( -1 );
	}
	if( libfcache_cache_get_number_of_entries(
	     cache,
	     &number_of_cache_entries,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of cache entries.",
		 function );

		return( -1 );
	}
	if( number_of_cache_entries <= 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid number of cache entries value out of bounds.",
		 function );

		return( -1 );
	}
	cache_entry_index = libfdata_vector_calculate_cache_entry_index(
	                     element_index,
	                     number_of_cache_entries );

	if( libfcache_cache_set_value_by_index(
	     cache,
	     cache_entry_index,
	     0,
	     element_data_offset,
	     internal_vector->timestamp,
	     element_value,
	     free_element_value,
	     flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set value in cache entry: %d.",
		 function,
		 cache_entry_index );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the element size of the vector
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_get_element_size(
     libfdata_vector_t *vector,
     size64_t *element_size,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_get_element_size";

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( element_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid element size.",
		 function );

		return( -1 );
	}
	*element_size = internal_vector->element_size;

	return( 1 );
}

/* Retrieves the data size of the vector
 * Returns 1 if successful or -1 on error
 */
int libfdata_vector_get_data_size(
     libfdata_vector_t *vector,
     size64_t *data_size,
     libcerror_error_t **error )
{
	libfdata_internal_vector_t *internal_vector = NULL;
	static char *function                       = "libfdata_vector_get_data_size";

	if( vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid vector.",
		 function );

		return( -1 );
	}
	internal_vector = (libfdata_internal_vector_t *) vector;

	if( data_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid data size.",
		 function );

		return( -1 );
	}
	*data_size = internal_vector->data_size;

	return( 1 );
}

