using System;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Security.Permissions;
using System.Text;
using Rl.Net.Native;

namespace Rl.Net
{
    [Serializable]
    public class RLException : Exception
    {
        private const string ErrorCodeSerializationKey = nameof(RLException.ErrorCode);

        public int ErrorCode
        {
            get;
            set;
        } = -1;

        public RLException(ApiStatus status) : base(status.ErrorMessage)
        {
            this.ErrorCode = status.ErrorCode;
        }

        public RLException(string message) : base(NativeMethods.OpaqueBindingErrorMessage + message)
        {
            this.ErrorCode = NativeMethods.OpaqueBindingError;
        }

        public RLException(ApiStatus status, Exception inner) : base(status.ErrorMessage, inner)
        {
            this.ErrorCode = status.ErrorCode;
        }

        protected RLException(SerializationInfo info, StreamingContext context) : base(info, context)
        {
            this.ErrorCode = info.GetInt32(ErrorCodeSerializationKey);
        }

        [SecurityPermission(SecurityAction.Demand, SerializationFormatter = true)]
        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            if (info == null)
            {
                throw new ArgumentNullException(nameof(info));
            }

            info.AddValue(ErrorCodeSerializationKey, this.ErrorCode);
            base.GetObjectData(info, context);
        } 

        [Conditional("DEBUG")]
        private static void WriteInnerExceptionOnDebug(ref string target, Exception outerException)
        {
            StringBuilder targetBuilder = new StringBuilder(target).AppendLine().AppendLine(outerException.ToString());
            target = targetBuilder.ToString();
        }

        internal int UpdateApiStatus(ApiStatus status)
        {
            string message = this.Message;
            WriteInnerExceptionOnDebug(ref message, this);

            ApiStatus.Update(status, this.ErrorCode, this.Message);
            return this.ErrorCode;
        }
    }
}